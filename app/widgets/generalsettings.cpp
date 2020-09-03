#include "generalsettings.h"
#include "ui_generalsettings.h"

#include "../classes/defaultSettings.h"

#include <QFileDialog>

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

double GeneralSettings::getMaxVal() const
{
    return maxVal;
}

void GeneralSettings::setMaxVal(double value)
{
    maxVal = value;

    ui->maxValSpinBox->setValue(value);
}

QString GeneralSettings::getPresetDir() const
{
    QString presetDir = ui->presetDirlineEdit->text();
    if (!presetDir.endsWith("/"))
        presetDir = presetDir + "/";

    return presetDir;
}

void GeneralSettings::setPresetDir(QString presetDirPath)
{
    QFileInfo presetDir(presetDirPath);
    ui->presetDirlineEdit->setText(presetDir.absolutePath());
}

void GeneralSettings::on_presetDirPushButton_clicked()
{
    QString presetDir = QFileDialog::getExistingDirectory(this, "Set preset folder", ui->presetDirlineEdit->text());
    if (presetDir != "")
        ui->presetDirlineEdit->setText(presetDir);
}

void GeneralSettings::on_defaultPushButton_clicked()
{
    ui->minValSpinBox->setValue(DEFAULT_LOWER_LIMIT);
    ui->maxValSpinBox->setValue(DEFAULT_UPPER_LIMIT);
    ui->useLimitsCheckBox->setCheckState(DEFAULT_USE_LIMITS ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->presetDirlineEdit->setText(QDir(DEFAULT_PRESET_DIR).absolutePath());
}
