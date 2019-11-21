#ifndef MVECTOR_H
#define MVECTOR_H

#include <QtCore>
#include <array>

#include "aclass.h"


class MVector
{

public:
    static const int size = 64;    // number of sensor inputs

    MVector();

    QString toString();

    bool operator ==(const MVector &other) const;

    bool operator !=(const MVector &other) const;

    /*
     * contains the sensor values measured
     */
    std::array<double, size> array;

    /*
     * class annotated by the user
     * -> can be used as base truth
     */
    aClass userDefinedClass{"",""};

    /*
     * automatically detected class
     */
    aClass detectedClass{"",""};

    /*
     * returns MVector with all elements being zero initialzed
     */
    static MVector zeroes();

};

#endif // MVECTOR_H
