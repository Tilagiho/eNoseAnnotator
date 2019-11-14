#include "generalsettings.h"
#include "ui_generalsettings.h"

GeneralSettings::GeneralSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralSettings)
{
    ui->setupUi(this);
    this->setWindowTitle("Settings");
}

GeneralSettings::~GeneralSettings()
{
    delete ui;
}



void GeneralSettings::on_buttonBox_accepted()
{
    maxVal = ui->maxValSpinBox->value();
    minVal = ui->minValSpinBox->value();
    saveRawInput = ui->saveRawInputCheckBox->checkState();
    useLimits = ui->useLimitsCheckBox->checkState();
}

bool GeneralSettings::getUseLimits() const
{
    return useLimits;
}

void GeneralSettings::setUseLimits(bool value)
{
    if (value)
        ui->useLimitsCheckBox->setCheckState(Qt::CheckState::Checked);
    else
        ui->useLimitsCheckBox->setCheckState(Qt::CheckState::Unchecked);

    useLimits = value;
}

double GeneralSettings::getMinVal() const
{
    return minVal;
}

void GeneralSettings::setMinVal(double value)
{
    ui->minValSpinBox->setValue(value);
    minVal = value;
}

bool GeneralSettings::getSaveRawInput() const
{
    return saveRawInput;
}

void GeneralSettings::setSaveRawInput(bool value)
{
    if (value)
        ui->saveRawInputCheckBox->setCheckState(Qt::CheckState::Checked);
    else
        ui->saveRawInputCheckBox->setCheckState(Qt::CheckState::Unchecked);
    saveRawInput = value;
}

double GeneralSettings::getMaxVal() const
{
    return maxVal;
}

void GeneralSettings::setMaxVal(double value)
{
    maxVal = value;

    ui->maxValSpinBox->setValue(value);
}
