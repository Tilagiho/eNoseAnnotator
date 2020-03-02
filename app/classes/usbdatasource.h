#ifndef USBDATASOURCE_H
#define USBDATASOURCE_H

#include "datasource.h"
#include "qserialport.h"

class USBDataSource : public DataSource
{
public:
    struct Settings{
        QString portName;

        // default QSerialPort settings
        QSerialPort::BaudRate baudRate = QSerialPort::Baud115200;
        QSerialPort::Parity parity = QSerialPort::Parity::NoParity;
        QSerialPort::DataBits dataBits = QSerialPort::Data8;
        QSerialPort::StopBits stopBits = QSerialPort::StopBits::OneStop;
        QSerialPort::FlowControl flowControl = QSerialPort::FlowControl::NoFlowControl;
    };

    USBDataSource(Settings settings);
    ~USBDataSource();

    Status status();

    SourceType sourceType();

    QString identifier();

    QSerialPort *getSerial() const;

    void reconnect();

public slots:
    void start();
    void pause();
    void stop();
    void reset();

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError serialPortError);
    void handleTimeout();
    void processLine(const QByteArray &line);

private:
    void openSerialPort();
    void closeSerialPort();
    void makeConnections();
    void closeConnections();

    QSerialPort *serial = nullptr;
    Settings settings;
    bool runningMeasFailed = false;

    bool emitData;
};

#endif // USBDATASOURCE_H
