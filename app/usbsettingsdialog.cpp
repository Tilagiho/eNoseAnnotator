#include "usbsettingsdialog.h"
#include "ui_usbsettingsdialog.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

USBSettingsDialog::USBSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::USBSettingsDialog)
{
    ui->setupUi(this);

    // show selected port info
    connect(ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &USBSettingsDialog::showPortInfo);

    fillPortsInfo();
}

USBSettingsDialog::~USBSettingsDialog()
{
    delete ui;
}

void USBSettingsDialog::showPortInfo(int idx)
{
    if (idx == -1)
        return;

    const QStringList list = ui->serialPortInfoListBox->itemData(idx).toStringList();
    ui->descriptionLabel->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    ui->serialNumberLabel->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    ui->locationLabel->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}


void USBSettingsDialog::fillPortsInfo()
{
    ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->serialPortInfoListBox->addItem(list.first(), list);
    }

    ui->serialPortInfoListBox->addItem(tr("Custom"));
}

USBSettingsDialog::Settings USBSettingsDialog::getSettings()
{
    return currentSettings;
}

bool USBSettingsDialog::setSettings(Settings settings)
{
    fillPortsInfo();

    // check if serialPortInfoListBox contains settings.portName
    QStringList itemsInComboBox;
    for (int index = 0; index < ui->serialPortInfoListBox->count(); index++)
        itemsInComboBox << ui->serialPortInfoListBox->itemText(index);

    if (!itemsInComboBox.contains(settings.portName))
        return false;

    ui->serialPortInfoListBox->setCurrentText(settings.portName);
    ui->sensorIdLineEdit->setText(settings.sensorId);



    currentSettings.portName = settings.portName;
    currentSettings.sensorId = settings.sensorId;

    return true;
}

void USBSettingsDialog::updateSettings()
{
    currentSettings.portName = ui->serialPortInfoListBox->currentText();
    currentSettings.sensorId = ui->sensorIdLineEdit->text();
}

void USBSettingsDialog::on_pushButton_clicked()
{
    updateSettings();

    // check sensorId
    if (currentSettings.sensorId == "")
    {
        QMessageBox::warning(this, "Settings not accepted", "Please set a sensorId!");
        return;
    }

    // close dialog as accepted
    accept();
}


USBSettingsDialog::Settings USBSettingsDialog::getDefaultSettings()
{
    Settings settings;

    settings.portName = "default";
    settings.sensorId = "default";

    return settings;
}
