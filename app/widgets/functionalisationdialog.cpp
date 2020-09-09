#include "functionalisationdialog.h"

#include "QInputDialog"
#include <QMessageBox>
#include <QObject>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

FunctionalisationDialog::FunctionalisationDialog(QWidget *parent, QString presetDir, ulong nChannels) :
    QDialog(parent),
    presetDir(presetDir),
    funcLabels(nChannels, nullptr),
    spinBoxes(nChannels, nullptr),
    nChannels(nChannels)
{
    // window title
    this->setWindowTitle("Set Functionalization");

    // presetLayout
    presetComboBox = new QComboBox();
    presetLabel = new QLabel("Presets:");
    loadPresetButton = new QPushButton("Load");
    savePresetButton = new QPushButton("Save");

    QHBoxLayout* presetButtonsLayout = new QHBoxLayout();
    presetButtonsLayout->addWidget(loadPresetButton);
    presetButtonsLayout->addWidget(savePresetButton);

    QHBoxLayout* presetHeaderLayout = new QHBoxLayout();
    presetHeaderLayout->addWidget(presetLabel);
    presetHeaderLayout->addWidget(presetComboBox);

    QVBoxLayout* presetLayout = new QVBoxLayout();
    presetLayout->addLayout(presetHeaderLayout);
    presetLayout->addLayout(presetButtonsLayout);

    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->addStretch(1);
    topLayout->addLayout(presetLayout);

    // set gridLayout
    QGridLayout* gridLayout = new QGridLayout();

    // setup funcLabels & spinBoxes
    for (uint i=0; i<spinBoxes.size(); i++)
    {
        funcLabels[i] = new QLabel("ch" + QString::number(i+1));
        spinBoxes[i] = new QSpinBox();
        // gridLayout looks like this
        // | label | spinBox | empty  | label | spinBox | empty | ... (spinBoxes.size() / 16 columns; end on spinBox)
        // | label | spinBox | empty  | label | spinBox | empty | ...
        // ... (16 rows)
        gridLayout->addWidget(funcLabels[i], i%16, 3 * (i/16));
        gridLayout->addWidget(spinBoxes[i], i%16, 3 * (i/16) + 1);

        QObject::connect(spinBoxes[i], QOverload<int>::of(&QSpinBox::valueChanged), this, &FunctionalisationDialog::valueChanged);
    }
    // add stretch to empty columns (every third colums is empty)
    for (uint i=0; i<=spinBoxes.size()/16; i++)
        gridLayout->setColumnStretch(3*i + 2, 1);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                        | QDialogButtonBox::Cancel
                                        | QDialogButtonBox::Reset);
    resetButton = buttonBox->button(QDialogButtonBox::Reset);

    QVBoxLayout* dialogLayout = new QVBoxLayout();
    dialogLayout->addLayout(topLayout);
    dialogLayout->addLayout(gridLayout);
    dialogLayout->addWidget(buttonBox);

    setLayout(dialogLayout);

    loadPresets();
    makeConnections();
}

FunctionalisationDialog::~FunctionalisationDialog()
{
    for (QLabel* label : funcLabels)
        label->deleteLater();
    for (QSpinBox* spinBox : spinBoxes)
        spinBox->deleteLater();

    presetLabel->deleteLater();
    loadPresetButton->deleteLater();
    savePresetButton->deleteLater();
    presetComboBox->deleteLater();
    buttonBox->deleteLater();
}

void FunctionalisationDialog::setFunctionalisation(std::vector<int> funcs)
{
    Q_ASSERT(funcs.size() == nChannels);

    for (uint i=0; i<nChannels; i++)
        spinBoxes[i]->setValue(funcs[i]);
}

std::vector<int> FunctionalisationDialog::getFunctionalisations()
{
    std::vector<int> funcs (nChannels, 0);

    for (uint i=0; i<nChannels; i++)
        funcs[i] = spinBoxes[i]->value();

    return funcs;
}

void FunctionalisationDialog::loadPresets()
{
    QDir directory(presetDir);
    QStringList presets = directory.entryList(QStringList() << "*.preset",QDir::Files);

    // defult: no preset
    presetComboBox->addItem("");

    for (QString presetFileName : presets)
    {
        // add preset to presetComboBox
        auto list = presetFileName.split(".");
        list.removeLast();
        QString preset = list.join(".");

        presetComboBox->addItem(preset);
    }
}

void FunctionalisationDialog::makeConnections()
{
    connect(presetComboBox, &QComboBox::currentTextChanged, this, [this](QString){
       updateLoadPresetButton();
    }); // update ok button when selected preset changes

    connect(buttonBox, &QDialogButtonBox::accepted, this, &FunctionalisationDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FunctionalisationDialog::reject);
    connect(resetButton, &QPushButton::released, this, &FunctionalisationDialog::resetSpinBoxes);   // reset spinBoxes
    connect(loadPresetButton, &QPushButton::released, this, &FunctionalisationDialog::loadSelectedPreset);   // load preset
    connect(savePresetButton, &QPushButton::released, this, &FunctionalisationDialog::savePreset);   // save preset
}

void FunctionalisationDialog::updateLoadPresetButton()
{
    bool presetSelected = presetComboBox->currentText() != "";
    loadPresetButton->setEnabled(presetSelected);
}

void FunctionalisationDialog::loadSelectedPreset()
{
    QString preset = presetComboBox->currentText();
    QString presetFileName = preset + ".preset";

    QFile file(presetDir + "/" + presetFileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, "Unable to open preset " + presetFileName,
            file.errorString());
    } else
    {
        QTextStream in(&file);

        QString line;
        std::vector<int> presetFuncs;
        bool readOk = true;
        int i = 0;
        while (true)
        {
            // load line & convert to integer
            readOk = in.readLineInto(&line);
            if (readOk)
                presetFuncs.push_back(line.toInt(&readOk));
            // EOF: leave loop
            else
                break;

            i++;
        }

        if (presetFuncs.size() == nChannels)
        {
            for (uint i = 0; i<spinBoxes.size(); i++)
                spinBoxes[i]->setValue(presetFuncs[i]);

            presetName = presetFileName;
        } else
        {
            QString message = presetFileName + " was created for " + QString::number(presetFuncs.size()) + " channels.\n" + QString::number(nChannels) + " channels expected.";
            QMessageBox::warning(this, "Error loading Preset", message);
        }
    }
}

void FunctionalisationDialog::resetSpinBoxes()
{
    for (uint i=0; i<spinBoxes.size(); i++)
        spinBoxes[i]->setValue(0);
}

void FunctionalisationDialog::savePreset()
{
    // create preset folder
    if(!QDir ("./presets").exists())
        QDir().mkdir("./presets");

    // get preset name
    QString input = QInputDialog::getText(this, "Save Preset", "Preset Name: ");

    if (input == "")
    {
        QMessageBox::critical(this, "Unable to save preset " + input, "Invalid preset name! ");
        return;
    }

    QString presetFileName = input + ".preset";

    QFile file("./presets/" + presetFileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, "Unable to save preset " + presetFileName,
            file.errorString());
    } else
    {
        QTextStream out(&file);

        for (uint i = 0; i<spinBoxes.size(); i++)
            out << QString::number(spinBoxes[i]->value()) << "\n";
    }
    // update preset combo box
    presetComboBox->addItem(input);
}

void FunctionalisationDialog::valueChanged(int)
{
    presetName = "Custom";

    // check if no functionalisation  was set
    for (uint i=0; i<spinBoxes.size(); i++)
        if (spinBoxes[i]->value() != 0)
            return;
    presetName = "None";
}
