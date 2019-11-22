#include "mvector.h"

#include <QtCore>

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

    stringList << userDefinedClass.toString() << detectedClass.toString();

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

double &MVector::operator[](int index)
{
    Q_ASSERT("index out of range!" && index >= 0 && index < size);

    return array[index];
}

MVector MVector::zeroes()
{
    return MVector();
}

MVector MVector::getRelativeVector(MVector baseVector)
{
    // cp vector data
    MVector deviationVector = *this;

    // calculate deviation / %
    for (int i=0; i<size; i++)
    {
        deviationVector[i] = 100 * (this->array[i] /  baseVector[i] - 1.0);
    }

    return deviationVector;
}

MVector MVector::getAbsoluteVector(MVector baseVector)
{
    // cp vector data
    MVector absoluteVector = *this;

    // calculate absolute resistances / Ohm
    for (int i=0; i<MVector::size; i++)
    {
        absoluteVector[i] = ((this->array[i] / 100.0) + 1.0) * baseVector[i];
    }
    return absoluteVector;
}
