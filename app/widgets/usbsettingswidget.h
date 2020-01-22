#ifndef USBSETTINGSWIDGET_H
#define USBSETTINGSWIDGET_H

#include <QtCore>
#include <QWidget>
#include <QtSerialPort/QSerialPort>

namespace Ui {
class USBSettingsWidget;
}

class USBSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit USBSettingsWidget(QWidget *parent = nullptr);
    ~USBSettingsWidget();

    QString getPortName();
    void setPortName(QString portName);

private:
    void showPortInfo(int idx);
    void fillPortsInfo();

    Ui::USBSettingsWidget *ui;
};

#endif // USBSETTINGSWIDGET_H
