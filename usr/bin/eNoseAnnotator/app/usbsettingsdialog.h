#ifndef USBSETTINGSDIALOG_H
#define USBSETTINGSDIALOG_H

#include <QtCore>
#include <QDialog>
#include <QtSerialPort/QSerialPort>

namespace Ui {
class USBSettingsDialog;
}

class USBSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    struct Settings {
        QString portName;
        QString sensorId;
        qint32 baudRate;
        QString stringBaudRate;
    };

    explicit USBSettingsDialog(QWidget *parent = nullptr);
    ~USBSettingsDialog();

    void showPortInfo(int idx);
    void fillPortsInfo();

    Settings getSettings();
    bool setSettings(Settings settings);
    void updateSettings();

    static Settings getDefaultSettings();

private slots:
    void on_pushButton_clicked();

private:
    Ui::USBSettingsDialog *ui;

    Settings currentSettings;
};

#endif // USBSETTINGSDIALOG_H
