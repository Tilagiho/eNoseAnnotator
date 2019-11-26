#include "generalsettings.h"
#include "ui_generalsettings.h"

GeneralSettings::GeneralSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralSettings)
{
    ui->setupUi(this);
    this->setWindowTitle("Settings");

    // BarGraphWidget::Mode
    QStringList modeList;
    modeList << "show functionalisation" << "show vector";
    ui->barGraphModecomboBox->addItems(modeList);
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
    showAbsGraph = ui->showAbsGraphCheckBox->checkState();

    // barGraphMode
    QString modeString = ui->barGraphModecomboBox->currentText();
    if (modeString == "show vector")
        barGraphMode = BarGraphWidget::Mode::showAll;
    else
        barGraphMode = BarGraphWidget::Mode::showFunc;
}

BarGraphWidget::Mode GeneralSettings::getBarGraphMode() const
{
    return barGraphMode;
}

void GeneralSettings::setBarGraphMode(const BarGraphWidget::Mode &value)
{
    barGraphMode = value;

    QString modeString;
    if (barGraphMode == BarGraphWidget::Mode::showAll)
        modeString = "show vector";
    else
        modeString = "show functionalisation";

    ui->barGraphModecomboBox->setCurrentText(modeString);
}

bool GeneralSettings::getShowAbsGraph() const
{
    return showAbsGraph;
}

void GeneralSettings::setShowAbsGraph(bool value)
{
    showAbsGraph = value;
    
    if (value)
        ui->showAbsGraphCheckBox->setCheckState(Qt::CheckState::Checked);
    else
        ui->showAbsGraphCheckBox->setCheckState(Qt::CheckState::Unchecked);
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
