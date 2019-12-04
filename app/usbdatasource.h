#ifndef USBDATASOURCE_H
#define USBDATASOURCE_H

#include <QtCore>
#include <QObject>
#include <QTimer>
#include "usbsettingsdialog.h"
#include "mvector.h"

class USBDataSource : public QObject
{
    Q_OBJECT
public:
    // constants
    const int nBaseLevel = 3;    // defines how many entries are used to set baseline after reset

    // enumeration for connection status
    enum class Status {
        NOT_CONNECTED,  // usb connection not open
        CONNECTED,
        SET_BASELEVEL,  // base level is being set
        RECEIVING_DATA,           // measurement data is being received
        CONNECTION_ERROR           // error occurred
    };


    explicit USBDataSource(QObject *parent = nullptr, USBSettingsDialog::Settings usbSettings = USBSettingsDialog::getDefaultSettings());
    ~USBDataSource();

    void changeSettings();

    void openSerialPort();
    void closeSerialPort();


    QSerialPort *getSerial() const;

    bool getPaused() const;

signals:
    void beginSetBaseLevel();
    void baseLevelSet(uint timestamp, MVector baseLevel);
    void vectorReceived(uint timestamp, MVector vector);
    void serialError();
    void serialTimeout();
    void newMeasurement(QString sensorId);

public slots:
    void setPaused(bool value);
    void reset();

private slots:
        void handleReadyRead();
    void handleError(QSerialPort::SerialPortError serialPortError);
    void handleTimeout();
    void processLine(const QByteArray &line);

private:
    void makeConnections();

    USBSettingsDialog *settings = nullptr;
    QSerialPort *serial = nullptr;
    QTimer timer;
    QMap<uint, MVector> baselevelVectorMap;
    int startCount = 0;     // used to store first count received in order to calculate baselevel vector
    Status status = Status::NOT_CONNECTED;
    bool paused = false;

    // default serial settings
    const QSerialPort::BaudRate dBaudRate = QSerialPort::Baud115200;
    const QSerialPort::Parity dParity = QSerialPort::Parity::NoParity;
    const QSerialPort::DataBits dDataBits = QSerialPort::Data8;
    const QSerialPort::StopBits dStopBits = QSerialPort::StopBits::OneStop;
    const QSerialPort::FlowControl dFlowControl = QSerialPort::FlowControl::NoFlowControl;
};

#endif // USBDATASOURCE_H
