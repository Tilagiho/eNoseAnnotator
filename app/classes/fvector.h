#ifndef FVECTOR_H
#define FVECTOR_H

#include <QtCore>
#include <array>

#include "aclass.h"
#include "annotation.h"


class FVector
{

public:
    int size = 1;    // number of functionalisations

    FVector(int n_funcs = 1);
    ~FVector();

    QString toString();

    bool operator ==(const FVector &other) const;

    bool operator !=(const FVector &other) const;

    FVector operator *(const double denominator);
    FVector operator *(const int denominator);
    FVector operator /(const double denominator);
    FVector operator /(const int denominator);

    FVector operator +(const FVector other);

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
    static FVector zeroes();

    /*
     * returns the deviation vector (/ %) of this relative to baseVector
     * WARNING: only use this function with an absolute vector
     */
    FVector getRelativeVector(FVector baseVector);

    /*
     * returns the absolute vector (/ Ohm) of this based on baseVector
     * WARNING: only use this function with relative vector
     */
    FVector getAbsoluteVector(FVector baseVector);
};

#endif // FVECTOR_H
