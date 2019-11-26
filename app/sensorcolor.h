#ifndef SENSORCOLOR_H
#define SENSORCOLOR_H

#include <QtCore>
#include <QColor>


class SensorColor
{
public:
    static QColor getSensorColor(int i);
    static QColor getFuncColor(int func, int funcSize);

private:
    SensorColor();
};

#endif // SENSORCOLOR_H
