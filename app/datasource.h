#ifndef DATASOURCE_H
#define DATASOURCE_H

#include "mvector.h"

class DataSource : public QObject
{
    Q_OBJECT
public:
    // used to obtain status of connection to source
    enum class Status {
        NOT_CONNECTED,  // usb connection not open
        CONNECTING, // trying to connect
        CONNECTED,  // connected, but not emitting data
        SET_BASEVECTOR,  // base level is being set
        RECEIVING_DATA,           // measurement data is being received & emitted
        CONNECTION_ERROR           // error occurred
    };

    // used to differentiate different source types
    enum class SourceType {
        USB,
        BLUETOOTH
    };

    // constants
    static const uint nBaseVectors;

    DataSource();

    virtual Status status();

    virtual void reconnect() = 0;

    virtual SourceType sourceType() = 0;
    virtual QString identifier() = 0;

signals:
    void vectorReceived (uint timestamp, MVector vector);
    void baseVectorSet (uint timestamp, MVector vector);

    void error(QString errorString);

    void statusSet(Status status);

public slots:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void reset() = 0;

protected:
    uint startCount = 0;
    Status connectionStatus = Status::NOT_CONNECTED;

    QTimer timer;   // used for emitting timeout errors

    QMap<uint, MVector> baselevelVectorMap; // used to store the first nBaseVectors vectors in order to calculate the base vector

    void setStatus(Status status);
};

#endif // DATASOURCE_H
