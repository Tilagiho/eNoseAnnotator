#include "usbsettingswidget.h"
#include "ui_usbsettingswidget.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

USBSettingsWidget::USBSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::USBSettingsWidget)
{
    ui->setupUi(this);

    // show selected port info
    connect(ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &USBSettingsWidget::showPortInfo);

    fillPortsInfo();
}

USBSettingsWidget::~USBSettingsWidget()
{
    delete ui;
}

void USBSettingsWidget::showPortInfo(int idx)
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


void USBSettingsWidget::fillPortsInfo()
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

QString USBSettingsWidget::getPortName()
{
    return ui->serialPortInfoListBox->currentText();
}

void USBSettingsWidget::setPortName(QString portName)
{
    // check if serialPortInfoListBox contains settings.portName
    QStringList itemsInComboBox;
    for (int index = 0; index < ui->serialPortInfoListBox->count(); index++)
        itemsInComboBox << ui->serialPortInfoListBox->itemText(index);

    // behaviour: portName not available -> ignore
    if (!itemsInComboBox.contains(portName))
        return;

    ui->serialPortInfoListBox->setCurrentText(portName);
}
