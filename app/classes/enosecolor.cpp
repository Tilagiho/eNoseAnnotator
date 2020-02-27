#include "enosecolor.h"

#include <math.h>
#include "measurementdata.h"
#include "mvector.h"

QList<QColor> ENoseColor::smallColorList {
    QColor("#006400"),  // darkgreen
    QColor("#1e90ff"),  // dodgerblue
    QColor("#ff0000"),  // red
    QColor("#ffd700"),  // gold
    QColor("#c71585"),  // mediumvioletred
    QColor("#00ff00"),  // lime
    QColor("#0000ff"),  // blue
    QColor("#7fffd4")   // aquamarine
};

QList<QColor> ENoseColor::bigColorList {
    QColor("#2f4f4f"),  // darkslategray
    QColor("#6b8e23"),  // olivedrab
    QColor("#7f0000"),  // maroon2
    QColor("#4b0082"),  // indigo
    QColor("#ff0000"),  // red
    QColor("#ffa500"),  // orange
    QColor("#c71585"),  // mediumvioletred
    QColor("#00ff00"),  // lime
    QColor("#00fa9a"),  // mediumspringgreen
    QColor("#e9967a"),  // darksalmon
    QColor("#00ffff"),  // aqua
    QColor("#0000ff"),  // blue
    QColor("#b0c4de"),  // lightsteelblue
    QColor("#ff00ff"),  // fuchsia
    QColor("#1e90ff"),  // dodgerblue
    QColor("#ffff54")   // laserlemon
};

ENoseColor::ENoseColor()
{}

QColor ENoseColor::getSensorColor(int ch)
{
    QColor color;

    // get
    auto functionalisation = MeasurementData::functionalisation;
    int maxFunc = 0;
    for (int i=0; i<MVector::nChannels; i++)
        if (functionalisation[i] > maxFunc)
            maxFunc = functionalisation[i];

    // pick color from list
    if (maxFunc < smallColorList.size())
        color = smallColorList[functionalisation[ch]];
    else if (maxFunc < bigColorList.size())
        color = bigColorList[functionalisation[ch]];
    // too many funcs: pick equally spaced color
    else
    {
        float hue = fmod(360/maxFunc * functionalisation[ch], 360.0);
        color.setHsv(hue, 200, 120);
    }

    return color;
}

QColor ENoseColor::getFuncColor(int func)
{
    QColor color;

    auto functionalisation = MeasurementData::functionalisation;
    int maxFunc = 0;
    for (int i=0; i<MVector::nChannels; i++)
        if (functionalisation[i] > maxFunc)
            maxFunc = functionalisation[i];

    // pick color from list
    if (maxFunc < smallColorList.size())
        color = smallColorList[func];
    else if (maxFunc < bigColorList.size())
        color = bigColorList[func];
    // too many funcs: pick equally spaced color
    else
    {
        float hue = fmod(360/maxFunc * func, 360.0);
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
        return
                getFuncColor(i);
}
