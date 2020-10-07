#include "enosecolor.h"

#include <math.h>
#include "measurementdata.h"
#include "mvector.h"

QColor ENoseColor::getSensorColor(int ch)
{
    QColor color;

    // get
    auto funcMap = MeasurementData::getFuncMap(functionalisation, sensorFailures);
    int funcSize = funcMap.size();

    int func = functionalisation[ch];

    // pick color from list
    if (funcSize < smallColorList.size())
        color = smallColorList[funcMap.keys().indexOf(func)];
    else if (funcSize < bigColorList.size())
        color = bigColorList[funcMap.keys().indexOf(func)];
    // too many funcs: pick equally spaced color
    else
    {
        float hue = fmod(360/funcSize * funcMap.keys().indexOf(func), 360.0);
        color.setHsv(hue, 200, 120);
    }

    return color;
}

QColor ENoseColor::getFuncColor(int index)
{
    QColor color;

    int funcSize = MeasurementData::getFuncMap(functionalisation, sensorFailures).size();

    // pick color from list
    if (funcSize < smallColorList.size())
        color = smallColorList[index];
    else if (funcSize < bigColorList.size())
        color = bigColorList[index];
    // too many funcs: pick equally spaced color
    else
    {
        float hue = fmod(360/funcSize * index, 360.0);
        color.setHsv(hue, 250, 150);
    }

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
    if (size < bigColorList.size())
        return bigColorList[i];
    else
        return getFuncColor(i);
}

void ENoseColor::setSensorFailures(const std::vector<bool> &value)
{
    sensorFailures = value;
}

void ENoseColor::setFunctionalisation(const std::vector<int> &value)
{
    functionalisation = value;
}
