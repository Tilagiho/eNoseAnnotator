#include "datasource.h"

/*!
 * \class DataSource
 * \brief Interface class for the connection with the eNoseSensor.
 * \ingroup classes
 * Currently implemented DataSources:
 * \list
 * \li USBDataSource
 * \endlist
 */

DataSource::DataSource(int sensorTimeout, int sensorNChannels):
    timeout(sensorTimeout),
    nChannels(sensorNChannels)
{
    qRegisterMetaType<Status>("Status");
    qRegisterMetaType<MVector>("MVector");
}

DataSource::~DataSource()
{
    timer->deleteLater();
}

/*!
 * \brief DataSource::nBaseVectors defines how many vectors are used to calculate the base vector (R0).
 * Vectors used to calculate base vectors are not added to MeasurementData.
 */
const uint DataSource::nBaseVectors = 3;

DataSource::Status DataSource::status()
{
    return connectionStatus;
}

bool DataSource::measIsRunning()
{
    return connectionStatus == DataSource::Status::RECEIVING_DATA || connectionStatus == DataSource::Status::SET_BASEVECTOR || connectionStatus == DataSource::Status::PAUSED;
}

int DataSource::getNChannels() const
{
    return nChannels;
}

int DataSource::getTimeout() const
{
    return timeout;
}

void DataSource::setTimeout(int value)
{
    timeout = value;
}

/*!
 * \brief DataSource::setStatus sets DataSource::connectionStatus to \a newStatus.
 * Emits \l {setStatus} if connectionStatus is changed.
 * \param newStatus
 */
void DataSource::setStatus(DataSource::Status newStatus)
{
    if (newStatus != connectionStatus)
    {
        connectionStatus = newStatus;
        emit statusSet(newStatus);
    }
}

void DataSource::started()
{
    timer = new QTimer();
    init();
}
