#ifndef FAKEDATASOURCE_H
#define FAKEDATASOURCE_H

#include "datasource.h"


class FakeDatasource : public DataSource
{
public:
    FakeDatasource(int timeout, int sensorNChannels);

    void reconnect();

    SourceType sourceType();
    QString identifier();

public slots:
    void start();
    void pause();
    void stop();
     void reset();


private slots:
    void setNextStatus();
    void emitMeasurement();

private:
    Status nextStatus;
    QTimer statusTimer{}, measTimer{};

    MVector generateMeasurement(double randRange=300.0);
};

#endif // FAKEDATASOURCE_H
