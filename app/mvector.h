#ifndef MVECTOR_H
#define MVECTOR_H

#include <QtCore>
#include <array>
#include <QtCore>


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
    QString userDefinedClass = "";

    /*
     * abreviation of the user defined class name
     * used in lineGraph
     */
    QString userDefinedClassBrief = "";

    /*
     * automatically detected class
     */
    QString detectedClass = "";

    /*
     * abreviation of the automatically detected class
     * should be as short as possible
     */
    QString detectedClassBrief = "";

    /*
     * returns MVector with all elements being zero initialzed
     */
    static MVector zeroes();

};

#endif // MVECTOR_H
