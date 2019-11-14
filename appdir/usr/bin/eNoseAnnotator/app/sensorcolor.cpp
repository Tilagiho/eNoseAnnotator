#include "sensorcolor.h"
#include <math.h>

SensorColor::SensorColor() {}

QColor SensorColor::getColor(int i)
{
    QColor color;
    float hue = fmod(45.625/4.0 * i, 360.0);
    color.setHsv(hue, 250, 150);

    return color;
}
