#ifndef DATASOURCE_H
#define DATASOURCE_H

#include "mvector.h"

class DataSource : public QObject
{
    Q_OBJECT
public:
    /*!
     * used to obtain status of connection to source
    */
    enum class Status {
        NOT_CONNECTED,  // usb connection not open
        CONNECTING, // trying to connect
        CONNECTED,  // connected, but not emitting data
        SET_BASEVECTOR,  // base level is being set
        RECEIVING_DATA,           // measurement data is being received & emitted
        PAUSED,
        CONNECTION_ERROR           // error occurred
    };

    // used to differentiate different source types
    enum class SourceType {
        USB,
        BLUETOOTH,
        FAKE
    };

    // constants
    static const uint nBaseVectors;

    DataSource(int sensorTimeout, int sensorNChannels);

    virtual Status status();

    virtual void reconnect() = 0;

    virtual SourceType sourceType() = 0;
    virtual QString identifier() = 0;


    int getNChannels() const;

    int getTimeout() const;
    void setTimeout(int value);

signals:
    /*! \fn void DataSource::vectorReceived(uint timestamp, MVector vector)
     *
       This signal is emitted if a new vector was received and the measurement was started before.
     */
    void vectorReceived (uint timestamp, MVector vector);

    /*! \fn void DataSource::baseVectorSet(uint timestamp, MVector vector)

       This signal is emitted after a new base vector was calculated. This happens at the start of a new measurement and after a reset was triggered.
     */
    void baseVectorSet (uint timestamp, MVector vector);

    /*! \fn void DataSource::error(QString errorString)

       This signal is emitted after a error in the connection appeared.
     */
    void error(QString errorString);

    /*! \fn void DataSource::statusSet(Status status)

       This signal is emitted after a new connection status was set.
     */
    void statusSet(Status status);

public slots:
    virtual void start() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void reset() = 0;

protected:
    /*!
     * \brief startCount stores the count of the first measurement vector received from the eNose sensorsince the last base level reset.
     */
    uint startCount = 0;

    Status connectionStatus = Status::NOT_CONNECTED;

    int timeout;

    int nChannels;

    /*!
     * \brief timer triggers error handling when timed out.
     * timer is reset every time a measurement was reveived. Times out when connection is broken.
     */
    QTimer timer;

    QMap<uint, MVector> baselevelVectorMap; // used to store the first nBaseVectors vectors in order to calculate the base vector

    void setStatus(Status status);
};

#endif // DATASOURCE_H
