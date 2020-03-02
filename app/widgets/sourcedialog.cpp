#include "sourcedialog.h"
#include "ui_sourcedialog.h"
#include "usbsettingswidget.h"

SourceDialog::SourceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SourceDialog),
    usbWidget(new USBSettingsWidget(this)),
    fakeWidget(new USBSettingsWidget(this))
{
    ui->setupUi(this);

    ui->applyButton->setEnabled(false);

    // input validator for sensorId line edit
    QRegExp rx("[\\w| ]*");
    ui->sensorIdLineEdit->setValidator(new QRegExpValidator(rx, this));

    // add source widgets
    ui->stackedWidget->addWidget(usbWidget);
    ui->stackedWidget->addWidget(fakeWidget);
    fakeWidget->setHidden(true);

    #ifdef QT_DEBUG
    ui->comboBox->addItem("Debug: Fake Data Source");
    ui->stackedWidget->setHidden(false);
    #endif
}

SourceDialog::~SourceDialog()
{
    delete ui;
    delete usbWidget;
}

QString SourceDialog::getIdentifier()
{
    return identifier;
}

void SourceDialog::setUSBSettings(USBDataSource::Settings usbSettings)
{
    usbWidget->setPortName(usbSettings.portName);
}

void SourceDialog::on_cancelButton_clicked()
{
    this->reject();
}

void SourceDialog::on_applyButton_clicked()
{
    sensorId = ui->sensorIdLineEdit->text();
    Q_ASSERT("sensorId cannot be empty!" && sensorId != "");

    if (ui->comboBox->currentText() == "USB")
    {
        sourceType = DataSource::SourceType::USB;
        identifier = usbWidget->getPortName();
    }
    else if (ui->comboBox->currentText() == "Debug: Fake Data Source")
    {
        sourceType = DataSource::SourceType::FAKE;
        identifier = "Fake";
    }
    else
        Q_ASSERT("Unknown source type selected!" && false);

    this->accept();
}

DataSource::SourceType SourceDialog::getSourceType() const
{
    return sourceType;
}

void SourceDialog::setSourceType(const DataSource::SourceType &value)
{
    sourceType = value;

    if (sourceType == DataSource::SourceType::USB)
        ui->stackedWidget->setCurrentWidget(usbWidget);
    else if (sourceType == DataSource::SourceType::FAKE)
        ui->stackedWidget->setCurrentWidget(fakeWidget);
}

void SourceDialog::on_sensorIdLineEdit_textEdited(const QString &newText)
{
    if (newText != "" && !ui->applyButton->isEnabled())
    {
        ui->applyButton->setEnabled(true);
        ui->applyButton->setToolTip("Apply data source changes.");
    }
    else if (newText == "" && ui->applyButton->isEnabled())
    {
        ui->applyButton->setEnabled(false);
        ui->applyButton->setToolTip("Apply data source changes.\nSensor Id cannot be empty.");
    }
}

void SourceDialog::on_comboBox_currentTextChanged(const QString &text)
{
    if (text == "USB")
    {
        ui->stackedWidget->setCurrentWidget(usbWidget);
        sourceType = DataSource::SourceType::USB;
    }
    if (text == "Debug: Fake Data Source")
    {
        ui->stackedWidget->setCurrentWidget(fakeWidget);
        sourceType = DataSource::SourceType::FAKE;
    }
    else
        Q_ASSERT("Unknown source type selected!" && false);
}

QString SourceDialog::getSensorId() const
{
    QString sensorId = ui->sensorIdLineEdit->text();
    Q_ASSERT("Sensor Id cannot be empty!" && sensorId != "");

    return sensorId;
}

void SourceDialog::setSensorId(const QString &value)
{
    sensorId = value;
    ui->sensorIdLineEdit->setText(value);
    ui->applyButton->setEnabled(true);
}
