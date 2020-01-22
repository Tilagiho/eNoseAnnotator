#include "datasource.h"

DataSource::DataSource()
{}

const uint DataSource::nBaseVectors = 3;

DataSource::Status DataSource::status()
{
    return connectionStatus;
}

void DataSource::setStatus(DataSource::Status newStatus)
{
    if (newStatus != connectionStatus)
    {
        connectionStatus = newStatus;
        emit statusSet(newStatus);
    }
}
