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
    ~MVector();

    QString toString();

    bool operator ==(const MVector &other) const;

    bool operator !=(const MVector &other) const;

    MVector operator *(const double denominator);
    MVector operator *(const int denominator);
    MVector operator /(const double denominator);
    MVector operator /(const int denominator);

    MVector operator +(const MVector other);

    // Overloading [] operator to access elements in array style
    double &operator[] (int index);

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

    /*
     * returns the deviation vector (/ %) of this relative to baseVector
     * WARNING: only use this function with an absolute vector
     */
    MVector toRelativeVector(MVector baseVector);

    /*
     * returns the absolute vector (/ Ohm) of this based on baseVector
     * WARNING: only use this function with relative vector
     */
    MVector getAbsoluteVector(MVector baseVector);
};

#endif // MVECTOR_H
