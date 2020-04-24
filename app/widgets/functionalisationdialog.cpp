#include "functionalisationdialog.h"
#include "ui_functionalisationdialog.h"

#include "QInputDialog"
#include <QMessageBox>
#include <QObject>

FunctionalisationDialog::FunctionalisationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FunctionalisationDialog),
    spinBoxes(64, nullptr)
{
    ui->setupUi(this);

    // window title
    this->setWindowTitle("Set Functionalization");

    // setup spArray
    spinBoxes[0] = ui->sp1;
    spinBoxes[1] = ui->sp2;
    spinBoxes[2] = ui->sp3;
    spinBoxes[3] = ui->sp4;
    spinBoxes[4] = ui->sp5;
    spinBoxes[5] = ui->sp6;
    spinBoxes[6] = ui->sp7;
    spinBoxes[7] = ui->sp8;
    spinBoxes[8] = ui->sp9;
    spinBoxes[9] = ui->sp10;
    spinBoxes[10] = ui->sp11;
    spinBoxes[11] = ui->sp12;
    spinBoxes[12] = ui->sp13;
    spinBoxes[13] = ui->sp14;
    spinBoxes[14] = ui->sp15;
    spinBoxes[15] = ui->sp16;
    spinBoxes[16] = ui->sp17;
    spinBoxes[17] = ui->sp18;
    spinBoxes[18] = ui->sp19;
    spinBoxes[19] = ui->sp20;
    spinBoxes[20] = ui->sp21;
    spinBoxes[21] = ui->sp22;
    spinBoxes[22] = ui->sp23;
    spinBoxes[23] = ui->sp24;
    spinBoxes[24] = ui->sp25;
    spinBoxes[25] = ui->sp26;
    spinBoxes[26] = ui->sp27;
    spinBoxes[27] = ui->sp28;
    spinBoxes[28] = ui->sp29;
    spinBoxes[29] = ui->sp30;
    spinBoxes[30] = ui->sp31;
    spinBoxes[31] = ui->sp32;
    spinBoxes[32] = ui->sp33;
    spinBoxes[33] = ui->sp34;
    spinBoxes[34] = ui->sp35;
    spinBoxes[35] = ui->sp36;
    spinBoxes[36] = ui->sp37;
    spinBoxes[37] = ui->sp38;
    spinBoxes[38] = ui->sp39;
    spinBoxes[39] = ui->sp40;
    spinBoxes[40] = ui->sp41;
    spinBoxes[41] = ui->sp42;
    spinBoxes[42] = ui->sp43;
    spinBoxes[43] = ui->sp44;
    spinBoxes[44] = ui->sp45;
    spinBoxes[45] = ui->sp46;
    spinBoxes[46] = ui->sp47;
    spinBoxes[47] = ui->sp48;
    spinBoxes[48] = ui->sp49;
    spinBoxes[49] = ui->sp50;
    spinBoxes[50] = ui->sp51;
    spinBoxes[51] = ui->sp52;
    spinBoxes[52] = ui->sp53;
    spinBoxes[53] = ui->sp54;
    spinBoxes[54] = ui->sp55;
    spinBoxes[55] = ui->sp56;
    spinBoxes[56] = ui->sp57;
    spinBoxes[57] = ui->sp58;
    spinBoxes[58] = ui->sp59;
    spinBoxes[59] = ui->sp60;
    spinBoxes[60] = ui->sp61;
    spinBoxes[61] = ui->sp62;
    spinBoxes[62] = ui->sp63;
    spinBoxes[63] = ui->sp64;


    for (QSpinBox* spinBox : spinBoxes)
        QObject::connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &FunctionalisationDialog::valueChanged);

    // presets
    loadPresets();
}

FunctionalisationDialog::~FunctionalisationDialog()
{
    delete ui;
}

void FunctionalisationDialog::setFunctionalities(std::vector<int> funcs)
{
    Q_ASSERT(funcs.size() == MVector::nChannels);

    for (int i=0; i<64; i++)
        spinBoxes[i]->setValue(funcs[i]);
}

std::vector<int> FunctionalisationDialog::getFunctionalities()
{
    std::vector<int> funcs (MVector::nChannels, 0);

    for (int i=0; i<MVector::nChannels; i++)
        funcs[i] = spinBoxes[i]->value();

    return funcs;
}

void FunctionalisationDialog::loadPresets()
{
    QDir directory("./presets");
    QStringList presets = directory.entryList(QStringList() << "*.preset",QDir::Files);

    for (QString presetFileName : presets)
    {
        auto list = presetFileName.split(".");
        list.removeLast();
        QString preset = list.join(".");

        ui->comboBox->addItem(preset);
    }
}

void FunctionalisationDialog::on_comboBox_currentTextChanged(const QString &arg1)
{
    if (arg1 == "")
        ui->pushButton->setEnabled(false);
    else
        ui->pushButton->setEnabled(true);
}

void FunctionalisationDialog::on_pushButton_clicked()
{
    QString preset = ui->comboBox->currentText();
    QString presetFileName = preset + ".preset";

    QFile file("./presets/" + presetFileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, "Unable to load preset " + presetFileName,
            file.errorString());
    } else
    {
        QTextStream in(&file);

        QString line;
        std::vector<int> presetArray (MVector::nChannels, 0);
        bool readOk = true;
        for (int i = 0; i<spinBoxes.size(); i++)
        {
            // load line & convert to integer
            readOk = in.readLineInto(&line);
            if (readOk)
                presetArray[i] = line.toInt(&readOk);
            if (!readOk)
            {
                QMessageBox::information(this, "Unable to load preset " + presetFileName,
                    file.errorString());
                break;
            }
        }

        if (readOk)
        {
            for (int i = 0; i<spinBoxes.size(); i++)
                spinBoxes[i]->setValue(presetArray[i]);

            presetName = presetFileName;
        }
    }
}

void FunctionalisationDialog::on_pushButton_2_clicked()
{
    for (int i=0; i<spinBoxes.size(); i++)
        spinBoxes[i]->setValue(0);
}

void FunctionalisationDialog::on_pushButton_3_clicked()
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

        for (int i = 0; i<spinBoxes.size(); i++)
            out << QString::number(spinBoxes[i]->value()) << "\n";
    }
    // update preset combo box
    ui->comboBox->addItem(input);
}

void FunctionalisationDialog::valueChanged(int)
{
    presetName = "Custom";

    // check if no functionalisation  was set
    for (int i=0; i<spinBoxes.size(); i++)
        if (spinBoxes[i]->value() != 0)
            return;
    presetName = "None";

}
