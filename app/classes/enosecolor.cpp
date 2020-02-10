#include "enosecolor.h"

#include <math.h>

ENoseColor::ENoseColor() {}

QColor ENoseColor::getSensorColor(int i)
{
    QColor color;
    float hue = fmod(45.625/4.0 * i, 360.0);
    color.setHsv(hue, 250, 150);

    return color;
}

QColor ENoseColor::getFuncColor(int func, int funcSize)
{
    QColor color;
    float hue = fmod(360/funcSize * func, 360.0);
    color.setHsv(hue, 250, 150);

    return color;
}

/*!
 * \brief ENoseColor::getClassColor returns QColor for ith of n classes
 * \param i
 * \param n
 * \return
 */
QColor ENoseColor::getClassColor(int i, int size)
{
    return getFuncColor(i, size);
}
