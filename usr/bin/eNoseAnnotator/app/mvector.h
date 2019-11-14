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

    std::array<double, size> array;

    /*
     * returns MVector with all elements being zero initialzed
     */
    static MVector zeroes();

};

#endif // MVECTOR_H
