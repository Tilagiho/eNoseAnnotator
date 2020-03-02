#include "usbdatasource.h"

#include <QtCore>

/*!
 * \class USBDataSource
 * \brief implements the DataSource interface for USB connections.
 */
USBDataSource::USBDataSource(USBDataSource::Settings settings):
    settings(settings)
{
    Q_ASSERT("Invalid settings. Serial port name has to be specified!" && settings.portName != "");

    serial = new QSerialPort();

    makeConnections();
    openSerialPort();
}

USBDataSource::~USBDataSource()
{
    closeConnections();
    closeSerialPort();
    delete serial;
}

DataSource::Status USBDataSource::status()
{
    if (connectionStatus != Status::NOT_CONNECTED)
    {
        // check if serial connection is still open
        if (!serial->isOpen())
        {
            closeSerialPort();
            setStatus (Status::CONNECTION_ERROR);
        }
    }

    return connectionStatus;
}

USBDataSource::SourceType USBDataSource::sourceType()
{
    return DataSource::SourceType::USB;
}

/*!
 * connect USBDataSource slots to QSerialPort signals
 */
void USBDataSource::makeConnections()
{
    connect(serial, &QSerialPort::readyRead, this, &USBDataSource::handleReadyRead);
    connect(serial, &QSerialPort::errorOccurred, this, &USBDataSource::handleError);
    connect(&timer, &QTimer::timeout, this, &USBDataSource::handleTimeout);
}

/*!
 * disconnect USBDataSource slots from QSerialPort signals
 */
void USBDataSource::closeConnections()
{
    disconnect(serial, &QSerialPort::readyRead, this, &USBDataSource::handleReadyRead);
    disconnect(serial, &QSerialPort::errorOccurred, this, &USBDataSource::handleError);
    disconnect(&timer, &QTimer::timeout, this, &USBDataSource::handleTimeout);
}

QSerialPort *USBDataSource::getSerial() const
{
    return serial;
}

void USBDataSource::openSerialPort()
{
    serial->setPortName(settings.portName);
    serial->setBaudRate(settings.baudRate);
    serial->setDataBits(settings.dataBits);
    serial->setParity(settings.parity);
    serial->setStopBits(settings.stopBits);
    serial->setFlowControl(settings.flowControl);

    // open connection
    if (serial->open(QIODevice::ReadOnly))
    {
        serial->clear();
        setStatus (DataSource::Status::CONNECTING);

        // don't emit data until measurement is started
        emitData = false;

        // set timer for timeout errors
        timer.setSingleShot(true);
        timer.start(5000);
    } else
    {
        emit error("Cannot open USB connection on port " + settings.portName);
        setStatus (Status::CONNECTION_ERROR);
    }
}

/*!
 * \brief USBDataSource::reconnect tries to reconnect the QSerialPort
 */
void USBDataSource::reconnect()
{
    if (serial != nullptr)
    {
        closeSerialPort();
        delete serial;
    }

    serial = new QSerialPort{};

    makeConnections();
    openSerialPort();
}

void USBDataSource::closeSerialPort()
{
    if (serial->isOpen())
    {
        serial->clear();
        serial->close();

        setStatus (Status::NOT_CONNECTED);

        timer.stop();
    }
}

/*!
 * triggered every time a vector is received from the eNose sensor
 */
void USBDataSource::handleReadyRead()
{
    // process lines
    while (serial->canReadLine())
        processLine(serial->readLine());
}

void USBDataSource::handleError(QSerialPort::SerialPortError serialPortError)
{
    // ignore signals other than read errors
    if (serialPortError != QSerialPort::SerialPortError::ReadError)
        return;
    if (status() == Status::RECEIVING_DATA)
        runningMeasFailed = true;

    closeSerialPort();

    setStatus (Status::CONNECTION_ERROR);
    emit error("An I/O error occurred while reading the data from USB port " + serial->portName() + ",  error: " + serial->errorString());
}

void USBDataSource::handleTimeout()
{
    if (connectionStatus == Status::CONNECTION_ERROR)
        return; // ignore if already in error state
    if (status() == Status::RECEIVING_DATA)
        runningMeasFailed = true;

    closeSerialPort();

    setStatus (Status::CONNECTION_ERROR);
    emit error("USB connection timed out without receiving data.\nCheck the connection settings and replug the sensor. Try to reconnect by starting a new measurement.");
}

/*!
 * \brief USBDataSource::processLine extracts a MVector from the message received.\n
 * if no measurement running: do nothing\n
 * if reset was triggered: use extracted MVector to calculate base vector. Emits base vector if no further vectors needed. \n
 * if measurement is running: emit extracted MVector
 */
void USBDataSource::processLine(const QByteArray &data)
{
    if (connectionStatus == DataSource::Status::CONNECTING)
    {
        if (runningMeasFailed)
        {
            runningMeasFailed = false;
            setStatus(DataSource::Status::PAUSED);
        }
        else
            setStatus(DataSource::Status::CONNECTED);
    }
    // reset timer
    timer.start(5000);

    if (!emitData)
        return;

    QString line(data);

//    qDebug() << line;
    if (line.startsWith("count"))
    {
        uint timestamp = QDateTime::currentDateTime().toTime_t();

        qDebug() << timestamp << ": Received new data";

        // line looks like: count=___,var1=____._,var2=____._,....
        QList<QString> dataList = line.split(',');
        QList<QString> valueList;

        for (auto element : dataList)
            valueList << element.split('=')[1];

        // extract values
        int count = valueList[0].toInt();

        if (startCount == 0)    // first count
        {
            baselevelVectorMap.clear();

            setStatus (Status::SET_BASEVECTOR);

            startCount = count;

            // reset base level:
            // add vector to baselevelVetorMap
            MVector vector;
            for (uint i=0; i<vector.size; i++)
                vector[i] = valueList[i+1].toDouble();

            baselevelVectorMap[timestamp] = vector;
        }
        else if (status() == Status::SET_BASEVECTOR && count < startCount + nBaseVectors -1) // prepare baselevel
        {
            MVector vector;
            for (uint i=0; i<vector.size; i++)
                vector[i] = valueList[i+1].toDouble();

            baselevelVectorMap[timestamp] = vector;
        } else if (status() == Status::SET_BASEVECTOR && count == startCount + nBaseVectors -1) // set baselevel
        {
            MVector vector;
            for (uint i=0; i<vector.size; i++)
                vector[i] = valueList[i+1].toDouble();

            baselevelVectorMap[timestamp] = vector;

            MVector baselevelVector = MVector::zeroes();

            for (uint ts : baselevelVectorMap.keys())
                baselevelVector = baselevelVector + baselevelVectorMap[ts] / baselevelVectorMap.size();

            // set base vector
            emit baseVectorSet(baselevelVectorMap.firstKey(), baselevelVector);

            // add data
//            for (uint ts : baselevelVectorMap.keys())
//                emit vectorReceived(ts, baselevelVectorMap[ts]);

        }
        else // get vector & emit
        {
            if (connectionStatus != Status::RECEIVING_DATA)
                setStatus (Status::RECEIVING_DATA);

            MVector vector;
            for (uint i=0; i<MVector::nChannels; i++)
                vector.vector[i] = valueList[i+1].toDouble();

//            qDebug() << "Vector Received: \n" << vector.toString();
            emit vectorReceived(timestamp, vector);
        }
    }
}

/*!
 * triggered to start emition of measurements
 */
void USBDataSource::start()
{
    Q_ASSERT("Usb connection was already started!" && connectionStatus != Status::RECEIVING_DATA);
    Q_ASSERT("Usb connection is not connected!" && connectionStatus != Status::NOT_CONNECTED);

    // start new measurement
    if (status() != Status::PAUSED && !baselevelVectorMap.isEmpty())
    {
        startCount = 0;
        setStatus(Status::SET_BASEVECTOR);
    }
    // resume existing measurement
    else
        setStatus(Status::RECEIVING_DATA);

    emitData = true;
}

/*!
 * triggered to pause emition of measurements
 */
void USBDataSource::pause()
{
    Q_ASSERT("Usb connection was already started!" && connectionStatus == Status::RECEIVING_DATA || connectionStatus == Status::SET_BASEVECTOR);

    if (connectionStatus == Status::SET_BASEVECTOR)
        baselevelVectorMap.clear();

    emitData = false;
    setStatus (Status::PAUSED);

}

/*!
 * triggers the end of vector emition
 */
void USBDataSource::stop()
{
    Q_ASSERT("Trying to stop connection that is not receiving data!" && (connectionStatus == Status::RECEIVING_DATA ||  connectionStatus == Status::PAUSED));

    emitData = false;
    runningMeasFailed = false;
    setStatus (Status::CONNECTED);
}

/*!
 * \brief USBDataSource::reset triggers a reset of the base vector
 */
void USBDataSource::reset()
{
    Q_ASSERT("Trying to reset base vector without receiving data!" && connectionStatus == Status::RECEIVING_DATA || connectionStatus == Status::PAUSED);

    emitData = true;
    startCount = 0;
    setStatus (Status::SET_BASEVECTOR);
}

/*!
 * \brief USBDataSource::identifier returns the port name of the current QSerialPort
 */
QString USBDataSource::identifier()
{
    return settings.portName;
}
