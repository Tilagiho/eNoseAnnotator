#ifndef SENSORCOLOR_H
#define SENSORCOLOR_H

#include <QtCore>
#include <QColor>


class SensorColor
{
public:
    static QColor getColor(int i);

private:
    SensorColor();
};

#endif // SENSORCOLOR_H
