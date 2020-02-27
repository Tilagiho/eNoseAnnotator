#ifndef SENSORCOLOR_H
#define SENSORCOLOR_H

#include <QtCore>
#include <QColor>

#include "annotation.h"

class ENoseColor
{
public:
    static QColor getSensorColor(int i);
    static QColor getFuncColor(int func);
    static QColor getClassColor(int i, int n);
    static QList<QColor> smallColorList;
    static QList<QColor> bigColorList;

private:
    ENoseColor();
};

#endif // SENSORCOLOR_H
