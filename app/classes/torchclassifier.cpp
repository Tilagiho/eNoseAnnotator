#include "torchclassifier.h"

#include <QtCore>

#include <iostream>
#include <memory>

TorchClassifier::TorchClassifier(QObject *parent, QString filename, bool* loadOk, QString* errorString, int nInputs):
    QObject(parent),
    filename(filename),
    N{nInputs}
{
    Q_ASSERT(loadOk != nullptr);

    *loadOk = true;

    // load mandatory attributes
    try
    {
    // Deserialize the ScriptModule from a file using torch::jit::load().
    module = torch::jit::load(filename.toStdString());
    }
    catch (const c10::Error& e)
    {
        *errorString += QString(e.msg().c_str());
        *loadOk = false;
        return;
    }

    // required attributes
    // extract class list
    if (!module.hasattr("classList"))
    {
        *loadOk = false;
        *errorString += "No class list provided with model.\n";
        return;
    }

    QString classListsString = module.attr("classList").toString().get()->string().c_str();
    classNames = classListsString.split(",");

    // load optional attributes
    // get model name
    if (module.hasattr("name"))
        name = QString(module.attr("name").toString()->string().c_str());
    else
        name = filename;

    // extract dimensions
    if (module.hasattr("N"))
        if (module.attr("N").toInt() != N)
        {
            *loadOk = false;
            *errorString += "Model takes " + QString::number(module.attr("N").toInt()) + " inputs, but " + QString::number(N) + " were provided!\n";
        }

    if (module.hasattr("M"))
        if (module.attr("M").toInt() != classNames.size())
        {
            *loadOk = false;
            *errorString += "Model is inconsistent: M was set to " + QString::number(module.attr("M").toInt()) + " outputs, but " + QString::number(classNames.size()) + " class names were provided!\n";
        }

    M = classNames.size();

    // input function
    if (module.hasattr("input_function"))
    {
        QString input_function = module.attr("input_function").toString()->string().c_str();

        if (input_function == "average")
            inputFunctionType = InputFunctionType::average;
        else if (input_function == "median_average")
            inputFunctionType = InputFunctionType::medianAverage;
        else if (input_function == "None")
            inputFunctionType = InputFunctionType::none;
    }

    // output function
    if (module.hasattr("output_function"))
    {
        QString output_function = module.attr("output_function").toString()->string().c_str();

        if (output_function == "logsoftmax")
            outputFunctionType = OutputFunctionType::logsoftmax;
        else if (output_function == "sigmoid")
            outputFunctionType = OutputFunctionType::sigmoid;
        else if (output_function == "None")
            outputFunctionType = OutputFunctionType::none;
    }

    // apply threshold
    if (module.hasattr("is_multi_label"))
        isMultiLabel = module.attr("is_multi_label").toBool();

    // threshold
    if (module.hasattr("threshold"))
        threshold = module.attr("threshold").toDouble();

    // input type
    if (module.hasattr("isInputAbsolute"))
        isInputAbsolute = module.attr("isInputAbsolute").toBool();

    // load normalisation values
    if (module.hasattr("mean"))
    {
        auto meanList = module.attr("mean").toDoubleList();

        for (auto it = meanList.begin(); it != meanList.end(); it++)
            mean_vector.push_back(*it);

        if (mean_vector.size() != N)
        {

            *loadOk = false;
            *errorString += "Model is inconsistent: Mean vector size is " + QString::number(mean_vector.size()) + " , input size N is " + QString::number(N) + "!\n";
            mean_vector.clear();
        }
    }

    if (module.hasattr("variance"))
    {
        auto varList = module.attr("variance").toDoubleList();

        for (auto it = varList.begin(); it != varList.end(); it++)
            stdev_vector.push_back(std::sqrt(*it));

        if (stdev_vector.size() != N)
        {

            *loadOk = false;
            *errorString += "Model is inconsistent: Variance vector size is " + QString::number(stdev_vector.size()) + " , input size N is " + QString::number(N) + "!\n";
            stdev_vector.clear();
        }
    }

    if (module.hasattr("preset_name"))
        presetName = QString(module.attr("preset_name").toString()->string().c_str());


//    qDebug() << module.dump_to_str(false, true, true, 3).c_str();
//    qDebug() << classNames.join(", ");

    if (!(*loadOk))
        *errorString += "See the classifier section <a href=\"https://github.com/Tilagiho/eNoseAnnotator/blob/master/README.md\">documentation</a> for more information.\n";
}

at::Tensor TorchClassifier::forward(std::vector<double> rawInput)
{
    // create float vector
    std::vector<float> floatVector;
    for (double value : rawInput)
        floatVector.push_back(static_cast<float>(value));
    // Create a vector of inputs.
    torch::Tensor inputTensor = at::from_blob(floatVector.data(), {1, 8});

//    std::cout << "Called forward with input: " << inputTensor.slice(1,0,8);
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(inputTensor);

    // Execute the model and turn its output into a tensor.
    at::Tensor output = module.forward(inputs).toTensor();

//    std::cout << "Calculated output: " << output.slice(1,0,classNames.size());


    return output;
}

Annotation TorchClassifier::getAnnotation(std::vector<double> input)
{
    if (input.size() != N)
        throw std::invalid_argument("Input vector has wrong size.");

//    std::cout << "input" << input << "\n";
    auto normalised_input = normalise(input);
//    std::cout << "normalised input" << normalised_input << "\n";

    // get class probabilities
    auto output = forward(normalised_input);

//    std::cout << "output: " << output.slice(1,0,classNames.size()) << "\n";

    // apply outputFunction
    at::Tensor probabilities;
    if (outputFunctionType == OutputFunctionType::logsoftmax)
        probabilities = torch::softmax(output, 1);
    else if (outputFunctionType == OutputFunctionType::sigmoid)
        probabilities = torch::sigmoid(output);
    else
        probabilities = output;

//    std::cout << "probabilities: " << probabilities.slice(1,0,classNames.size()) << "\n";


    //                              //
    // extract classes from tensor  //
    //                              //
    QSet<aClass> classSet;

    float* ptr = (float*) probabilities.data_ptr();
    for (int i = 0; i < classNames.size(); ++i)
        classSet << aClass(classNames[i], *ptr++);

    //                                          //
    // make predictions based on probabilities  //
    //                                          //
    QSet<aClass> predClasses;
    // multi label classification:
    if (isMultiLabel)
    {
        // apply threshold to get labels
        for (aClass aclass : classSet)
        {
            if (aclass.getValue() > threshold)
                predClasses << aClass(aclass);
        }
        // set prob of No Smell 1-maxProb
        double maxProb = 0.;
        for (aClass aclass : classSet)
        {
            if (aclass.getValue() > maxProb)
                maxProb = aclass.getValue();
        }
        classSet << aClass(NO_SMELL_STRING, 1.-maxProb);

        // set pred to No Smell if no label was detected
        if (predClasses.isEmpty())
            predClasses << aClass(NO_SMELL_STRING);
    }
    // multi-class classification:
    // get class with highest probabillity
    else
    {
        for (aClass aclass : classSet)
            if (predClasses.isEmpty())
                predClasses << aclass;
            else if (aclass.getValue() > predClasses.begin()->getValue())
            {
                predClasses.clear();
                predClasses << aClass(aclass);
            }
    }
    // no regression:
    // ignore value of predicted classes
    if (!isRegression)
    {
        for (aClass aclass : predClasses)
            aclass.setType(aClass::Type::CLASS_ONLY);
    }

    return Annotation(classSet, predClasses);
}

QString TorchClassifier::getName() const
{
    return name;
}

QString TorchClassifier::getFilename() const
{
    return filename;
}

bool TorchClassifier::getIsInputAbsolute() const
{
    return isInputAbsolute;
}

QStringList TorchClassifier::getClassNames() const
{
    return classNames;
}

int TorchClassifier::getN() const
{
    return N;
}

int TorchClassifier::getM() const
{
    return M;
}

QString TorchClassifier::getPresetName() const
{
    return presetName;
}

InputFunctionType TorchClassifier::getInputFunctionType() const
{
    return inputFunctionType;
}

std::vector<double> TorchClassifier::normalise(std::vector<double> input)
{
    Q_ASSERT(input.size() == N);

    // variance not set:
    // don't normalise
    if (stdev_vector.empty())
        return input;

    // mean not set:
    // zero init
    if (mean_vector.empty())
        for (int i=0; i<N; i++)
            mean_vector.push_back(0.0);

    Q_ASSERT(mean_vector.size() == N);
    Q_ASSERT(stdev_vector.size() == N);

    std::vector<double> normalised_vector;
    for (uint i=0; i<N; i++)
        normalised_vector.push_back((input[i] - mean_vector[i]) / stdev_vector[i]);

    return normalised_vector;

}
