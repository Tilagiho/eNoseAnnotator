#include "setsensorfailuresdialog.h"
#include "ui_setsensorfailuresdialog.h"

#include <QtCore>
#include <QtGui>
#include <QDialog>

#include "../classes/measurementdata.h"

SetSensorFailuresDialog::SetSensorFailuresDialog(QWidget *parent, QString failureString) :
    QDialog(parent),
    ui(new Ui::SetSensorFailuresDialog)
{
    auto failureArray = MeasurementData::sensorFailureArray(failureString);

    ui->setupUi(this);

    // set up checkbox array
    checkBoxArray[0] = ui->checkBox;
    checkBoxArray[1] = ui->checkBox_2;
    checkBoxArray[2] = ui->checkBox_3;
    checkBoxArray[3] = ui->checkBox_4;
    checkBoxArray[4] = ui->checkBox_5;
    checkBoxArray[5] = ui->checkBox_6;
    checkBoxArray[6] = ui->checkBox_7;
    checkBoxArray[7] = ui->checkBox_8;
    checkBoxArray[8] = ui->checkBox_9;
    checkBoxArray[9] = ui->checkBox_10;
    checkBoxArray[10] = ui->checkBox_11;
    checkBoxArray[11] = ui->checkBox_12;
    checkBoxArray[12] = ui->checkBox_13;
    checkBoxArray[13] = ui->checkBox_14;
    checkBoxArray[14] = ui->checkBox_15;
    checkBoxArray[15] = ui->checkBox_16;
    checkBoxArray[16] = ui->checkBox_17;
    checkBoxArray[17] = ui->checkBox_18;
    checkBoxArray[18] = ui->checkBox_19;
    checkBoxArray[19] = ui->checkBox_20;
    checkBoxArray[20] = ui->checkBox_21;
    checkBoxArray[21] = ui->checkBox_22;
    checkBoxArray[22] = ui->checkBox_23;
    checkBoxArray[23] = ui->checkBox_24;
    checkBoxArray[24] = ui->checkBox_25;
    checkBoxArray[25] = ui->checkBox_26;
    checkBoxArray[26] = ui->checkBox_27;
    checkBoxArray[27] = ui->checkBox_28;
    checkBoxArray[28] = ui->checkBox_29;
    checkBoxArray[29] = ui->checkBox_30;
    checkBoxArray[30] = ui->checkBox_31;
    checkBoxArray[31] = ui->checkBox_32;
    checkBoxArray[32] = ui->checkBox_33;
    checkBoxArray[33] = ui->checkBox_34;
    checkBoxArray[34] = ui->checkBox_35;
    checkBoxArray[35] = ui->checkBox_36;
    checkBoxArray[36] = ui->checkBox_37;
    checkBoxArray[37] = ui->checkBox_38;
    checkBoxArray[38] = ui->checkBox_39;
    checkBoxArray[39] = ui->checkBox_40;
    checkBoxArray[40] = ui->checkBox_41;
    checkBoxArray[41] = ui->checkBox_42;
    checkBoxArray[42] = ui->checkBox_43;
    checkBoxArray[43] = ui->checkBox_44;
    checkBoxArray[44] = ui->checkBox_45;
    checkBoxArray[45] = ui->checkBox_46;
    checkBoxArray[46] = ui->checkBox_47;
    checkBoxArray[47] = ui->checkBox_48;
    checkBoxArray[48] = ui->checkBox_49;
    checkBoxArray[49] = ui->checkBox_50;
    checkBoxArray[50] = ui->checkBox_51;
    checkBoxArray[51] = ui->checkBox_52;
    checkBoxArray[52] = ui->checkBox_53;
    checkBoxArray[53] = ui->checkBox_54;
    checkBoxArray[54] = ui->checkBox_55;
    checkBoxArray[55] = ui->checkBox_56;
    checkBoxArray[56] = ui->checkBox_57;
    checkBoxArray[57] = ui->checkBox_58;
    checkBoxArray[58] = ui->checkBox_59;
    checkBoxArray[59] = ui->checkBox_60;
    checkBoxArray[60] = ui->checkBox_61;
    checkBoxArray[61] = ui->checkBox_62;
    checkBoxArray[62] = ui->checkBox_63;
    checkBoxArray[63] = ui->checkBox_64;

    // set failure states
    for (int i=0; i<MVector::size; i++)
    {
        if (failureArray[i])
        {
            checkBoxArray[i]->setCheckState(Qt::CheckState::Checked);
            sensorFailures[i] = true;
        } else
        {
            checkBoxArray[i]->setCheckState(Qt::CheckState::Unchecked);
            sensorFailures[i] = false;
        }
    }
}

SetSensorFailuresDialog::~SetSensorFailuresDialog()
{
    delete ui;
}

std::array<bool, 64> SetSensorFailuresDialog::getSensorFailures()
{
    return sensorFailures;
}

void SetSensorFailuresDialog::on_buttonBox_accepted()
{
    // get bits
    for (int i=0; i<MVector::size; i++)
    {
        sensorFailures[i] = checkBoxArray[i]->checkState() == Qt::CheckState::Checked;
    }

    this->close();
}

void SetSensorFailuresDialog::on_ResetButton_clicked()
{
    for (int i=0; i<MVector::size; i++)
        checkBoxArray[i]->setCheckState(Qt::CheckState::Unchecked);
}
