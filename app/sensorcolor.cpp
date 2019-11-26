#include "sensorcolor.h"
#include <math.h>

SensorColor::SensorColor() {}

QColor SensorColor::getSensorColor(int i)
{
    QColor color;
    float hue = fmod(45.625/4.0 * i, 360.0);
    color.setHsv(hue, 250, 150);

    return color;
}

QColor SensorColor::getFuncColor(int func, int funcSize)
{
    QColor color;
    float hue = fmod(360/funcSize * func, 360.0);
    color.setHsv(hue, 250, 150);

    return color;
}
