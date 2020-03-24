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

    try
    {
    // Deserialize the ScriptModule from a file using torch::jit::load().
    module = torch::jit::load(filename.toStdString());
    }
    catch (const c10::Error& e)
    {
        qDebug() << QString(e.msg().c_str());
        *loadOk = false;
        return;
    }

    // extract class list
    if (!module.hasattr("classList"))
    {
        *loadOk = false;
        *errorString += "No class list provided with model.\n";
    }
    QString classListsString = (module.attr("classList").toString().get()->string().c_str());
    classNames = classListsString.split(",");

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
        *errorString += "See the <a href=\"https://github.com/Tilagiho/eNoseAnnotator/blob/master/README.md\">documentation</a> for more information.\n";
}

at::Tensor TorchClassifier::forward(std::vector<double> rawInput, bool asChances)
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

    auto probabilities = torch::softmax(output, 1);

//    std::cout << "probabilities: " << probabilities.slice(1,0,classNames.size()) << "\n";


    // extract classes from tensor
    QSet<aClass> classSet;

    float* ptr = (float*) probabilities.data_ptr();
    for (int i = 0; i < classNames.size(); ++i)
        classSet << aClass(classNames[i], *ptr++);

    return Annotation(classSet);
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
