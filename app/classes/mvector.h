#ifndef MVECTOR_H
#define MVECTOR_H

#include <QtCore>
#include <array>

#include "aclass.h"
#include "annotation.h"

class MVector
{

public:
    static const int nChannels = 64;

    MVector(int size=nChannels);
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
    std::vector<double> vector;

    /*
     * class annotated by the user
     * -> can be used as base truth
     */
    Annotation userAnnotation;

    /*
     * automatically detected class
     */
    Annotation detectedAnnotation;

    /*
     * returns MVector with all elements being zero initialzed
     */
    static MVector zeroes();

    /*
     * returns the deviation vector (/ %) of this relative to baseVector
     * WARNING: only use this function with an absolute vector
     */
    MVector getRelativeVector(MVector baseVector);

    /*
     * returns the absolute vector (/ Ohm) of this based on baseVector
     * WARNING: only use this function with relative vector
     */
    MVector getAbsoluteVector(MVector baseVector);

    MVector getFuncVector(std::array<int, MVector::nChannels> functionalisation, std::array<bool, MVector::nChannels> sensorFailures);

    int size = nChannels;    // number of sensor inputs
};

#endif // MVECTOR_H
