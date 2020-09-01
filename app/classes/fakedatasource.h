#ifndef FAKEDATASOURCE_H
#define FAKEDATASOURCE_H

#include "datasource.h"


class FakeDatasource : public DataSource
{
public:
    FakeDatasource(int timeout, int sensorNChannels);
    ~FakeDatasource();

    void reconnect() override;

    SourceType sourceType() override;
    QString identifier() override;

public slots:
    void init() override;
    void start() override;
    void pause() override;
    void stop() override;
    void reset() override;


private slots:
    void setNextStatus();
    void emitMeasurement();

private:
    Status nextStatus;
    QTimer* statusTimer = nullptr;
    QTimer* measTimer = nullptr;

    MVector generateMeasurement(double randRange=300.0);
};

#endif // FAKEDATASOURCE_H
