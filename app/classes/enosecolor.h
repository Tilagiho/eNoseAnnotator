#ifndef SENSORCOLOR_H
#define SENSORCOLOR_H

#include <QtCore>
#include <QColor>

#include "annotation.h"

class ENoseColor
{
public:
    static QColor getSensorColor(int i);
    static QColor getFuncColor(int func, int funcSize);
    static QColor getClassColor(int i, int n);

private:
    ENoseColor();
};

#endif // SENSORCOLOR_H
