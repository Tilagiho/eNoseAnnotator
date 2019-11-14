#include "mvector.h"

MVector::MVector()
{
    // init array
    for (int i=0; i<size; i++)
        array[i] = 0.0;
}

QString MVector::toString()
{
    QStringList stringList;

    for (int i=0; i<size; i++)
        stringList << QString::number(array[i]);

    return stringList.join(";");
}

bool MVector::operator==(const MVector &other) const
{
    double epsilon = 0.0001;

    // this == other, if ||this->array[i]-other.array[i]|| < epsilon for i<size
    for (int i=0; i<size; i++)
        if (qAbs(this->array[i]-other.array[i]) > epsilon)
            return false;

    return true;
}

bool MVector::operator!=(const MVector &other) const
{
    // not equal
    return !this->operator==(other);
}

MVector MVector::zeroes()
{
    return MVector();
}
