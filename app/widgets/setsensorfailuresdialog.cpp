#include "setsensorfailuresdialog.h"
#include "ui_setsensorfailuresdialog.h"

#include <QtCore>
#include <QtGui>
#include <QDialog>

#include "../classes/measurementdata.h"

SetSensorFailuresDialog::SetSensorFailuresDialog(QWidget *parent, QString failureString) :
    QDialog(parent),
    ui(new Ui::SetSensorFailuresDialog),
    checkBoxes(64, nullptr),
    sensorFailures(MVector::nChannels, false)
{
    auto failures = MeasurementData::sensorFailureArray(failureString);

    ui->setupUi(this);

    // set up checkbox array
    checkBoxes[0] = ui->checkBox;
    checkBoxes[1] = ui->checkBox_2;
    checkBoxes[2] = ui->checkBox_3;
    checkBoxes[3] = ui->checkBox_4;
    checkBoxes[4] = ui->checkBox_5;
    checkBoxes[5] = ui->checkBox_6;
    checkBoxes[6] = ui->checkBox_7;
    checkBoxes[7] = ui->checkBox_8;
    checkBoxes[8] = ui->checkBox_9;
    checkBoxes[9] = ui->checkBox_10;
    checkBoxes[10] = ui->checkBox_11;
    checkBoxes[11] = ui->checkBox_12;
    checkBoxes[12] = ui->checkBox_13;
    checkBoxes[13] = ui->checkBox_14;
    checkBoxes[14] = ui->checkBox_15;
    checkBoxes[15] = ui->checkBox_16;
    checkBoxes[16] = ui->checkBox_17;
    checkBoxes[17] = ui->checkBox_18;
    checkBoxes[18] = ui->checkBox_19;
    checkBoxes[19] = ui->checkBox_20;
    checkBoxes[20] = ui->checkBox_21;
    checkBoxes[21] = ui->checkBox_22;
    checkBoxes[22] = ui->checkBox_23;
    checkBoxes[23] = ui->checkBox_24;
    checkBoxes[24] = ui->checkBox_25;
    checkBoxes[25] = ui->checkBox_26;
    checkBoxes[26] = ui->checkBox_27;
    checkBoxes[27] = ui->checkBox_28;
    checkBoxes[28] = ui->checkBox_29;
    checkBoxes[29] = ui->checkBox_30;
    checkBoxes[30] = ui->checkBox_31;
    checkBoxes[31] = ui->checkBox_32;
    checkBoxes[32] = ui->checkBox_33;
    checkBoxes[33] = ui->checkBox_34;
    checkBoxes[34] = ui->checkBox_35;
    checkBoxes[35] = ui->checkBox_36;
    checkBoxes[36] = ui->checkBox_37;
    checkBoxes[37] = ui->checkBox_38;
    checkBoxes[38] = ui->checkBox_39;
    checkBoxes[39] = ui->checkBox_40;
    checkBoxes[40] = ui->checkBox_41;
    checkBoxes[41] = ui->checkBox_42;
    checkBoxes[42] = ui->checkBox_43;
    checkBoxes[43] = ui->checkBox_44;
    checkBoxes[44] = ui->checkBox_45;
    checkBoxes[45] = ui->checkBox_46;
    checkBoxes[46] = ui->checkBox_47;
    checkBoxes[47] = ui->checkBox_48;
    checkBoxes[48] = ui->checkBox_49;
    checkBoxes[49] = ui->checkBox_50;
    checkBoxes[50] = ui->checkBox_51;
    checkBoxes[51] = ui->checkBox_52;
    checkBoxes[52] = ui->checkBox_53;
    checkBoxes[53] = ui->checkBox_54;
    checkBoxes[54] = ui->checkBox_55;
    checkBoxes[55] = ui->checkBox_56;
    checkBoxes[56] = ui->checkBox_57;
    checkBoxes[57] = ui->checkBox_58;
    checkBoxes[58] = ui->checkBox_59;
    checkBoxes[59] = ui->checkBox_60;
    checkBoxes[60] = ui->checkBox_61;
    checkBoxes[61] = ui->checkBox_62;
    checkBoxes[62] = ui->checkBox_63;
    checkBoxes[63] = ui->checkBox_64;

    // set failure states
    for (int i=0; i<MVector::nChannels; i++)
    {
        if (failures[i])
        {
            checkBoxes[i]->setCheckState(Qt::CheckState::Checked);
            sensorFailures[i] = true;
        } else
        {
            checkBoxes[i]->setCheckState(Qt::CheckState::Unchecked);
            sensorFailures[i] = false;
        }
    }
}

SetSensorFailuresDialog::~SetSensorFailuresDialog()
{
    delete ui;
}

std::vector<bool> SetSensorFailuresDialog::getSensorFailures()
{
    return sensorFailures;
}

void SetSensorFailuresDialog::on_buttonBox_accepted()
{
    // get bits
    for (int i=0; i<MVector::nChannels; i++)
    {
        sensorFailures[i] = checkBoxes[i]->checkState() == Qt::CheckState::Checked;
    }

    this->close();
}

void SetSensorFailuresDialog::on_ResetButton_clicked()
{
    for (int i=0; i<MVector::nChannels; i++)
        checkBoxes[i]->setCheckState(Qt::CheckState::Unchecked);
}
