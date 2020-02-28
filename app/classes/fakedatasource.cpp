#include "fakedatasource.h"

#include <QRandomGenerator>

/*!
 * \brief FakeDatasource::FakeDatasource generate random data.
 */
FakeDatasource::FakeDatasource()
{
    connect(&statusTimer, &QTimer::timeout, this, &FakeDatasource::setNextStatus);
    connect(&measTimer, &QTimer::timeout, this, &FakeDatasource::emitMeasurement);

    setStatus(Status::CONNECTING);
    nextStatus = Status::CONNECTED;

    // set timer
    statusTimer.setSingleShot(true);
    statusTimer.start(3000);
}

void FakeDatasource::setNextStatus()
{
    Status oldStatus = status();
    Q_ASSERT(oldStatus != nextStatus);

    setStatus(nextStatus);

    switch (oldStatus)
    {
    case Status::NOT_CONNECTED:
        // not connected -> connecting:
        //      connecting -> connected
        if (nextStatus == Status::CONNECTING)
        {
            nextStatus = Status::CONNECTED;
            statusTimer.start(3000);
        }
        break;
    case Status::CONNECTING:
        // connecting -> set basevector:
        //      do nothing
        if (nextStatus == Status::CONNECTED)
            ;
        else if (nextStatus == Status::RECEIVING_DATA)
        {
            nextStatus = Status::CONNECTION_ERROR;
            statusTimer.start(30000);
            measTimer.start(2000);
        }
        break;
    case Status::CONNECTED:
        if (nextStatus == Status::SET_BASEVECTOR)
        {
            nextStatus = Status::RECEIVING_DATA;
            statusTimer.start(3000);
        }
        break;
    case Status::SET_BASEVECTOR:
        // set basevector -> receiving data:
        //      emit base vector, receiving data -> error
        if (nextStatus == Status::RECEIVING_DATA)
        {
            emit baseVectorSet(QDateTime::currentDateTime().toTime_t(), generateMeasurement(50.0));
            nextStatus = Status::CONNECTION_ERROR;
            statusTimer.start(30000);
            measTimer.start(2000);
        }
        break;
    case Status::RECEIVING_DATA:
        // measurement -> error:
        //      stop measurement timer
        if (nextStatus == Status::CONNECTION_ERROR)
        {
            measTimer.stop();
        }
        break;
    }

}

MVector FakeDatasource::generateMeasurement(double randRange)
{
    MVector vector;

    for (int i=0; i<MVector::nChannels; i++)
        vector[i] = 1000.0 + 50.0*i + randRange*(QRandomGenerator::global()->generateDouble() - 0.5);
    return vector;
}

void FakeDatasource::emitMeasurement()
{
    MVector vector = generateMeasurement();

    emit vectorReceived(QDateTime::currentDateTime().toTime_t(), vector);
}

void FakeDatasource::start()
{
    Q_ASSERT(status() == Status::CONNECTED || status() == Status::PAUSED);

    if (status() == Status::PAUSED)
        setStatus(Status::CONNECTING);
    else
        setStatus(Status::SET_BASEVECTOR);
    statusTimer.start(3000);
    nextStatus = Status::RECEIVING_DATA;
}

void FakeDatasource::pause()
{
    Q_ASSERT(status() == Status::RECEIVING_DATA);

    measTimer.stop();
    setStatus(Status::PAUSED);
}

void FakeDatasource::reset()
{
    Q_ASSERT(status() == Status::RECEIVING_DATA || status() == Status::PAUSED);

    if (status() == Status::RECEIVING_DATA)
        measTimer.stop();

    setStatus(Status::SET_BASEVECTOR);
    nextStatus = Status::RECEIVING_DATA;
    statusTimer.start(3000);
}

void FakeDatasource::stop()
{
    Q_ASSERT(status() == Status::RECEIVING_DATA || status() == Status::PAUSED);

    measTimer.stop();
    statusTimer.stop();
    setStatus(Status::CONNECTED);
}

void FakeDatasource::reconnect()
{
    Q_ASSERT(status() == Status::CONNECTION_ERROR);

    setStatus(Status::CONNECTING);
    nextStatus = Status::RECEIVING_DATA;
    statusTimer.start(3000);
}

QString FakeDatasource::identifier()
{
    return "Fake Data Source";
}

DataSource::SourceType FakeDatasource::sourceType()
{
   return SourceType::FAKE;
}
