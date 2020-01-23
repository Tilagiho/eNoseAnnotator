#include "mvector.h"

#include <QtCore>

/*!
  \class MVector
  \brief The MVector class represents one measurement vector of the eNose sensor.
  \ingroup classes
  The size measurement values received every two seconds from the eNose sensor can be stored in MVectors.
  MVector can contain absolute resistance values, but also deviations relative to a base resistance (R0).
*/
MVector::MVector()
{
    // init array
    for (int i=0; i<size; i++)
        array[i] = 0.0;
}

MVector::~MVector()
{

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

MVector MVector::operator*(const double multiplier)
{
    MVector vector;

    for (int i=0; i<size; i++)
        vector[i] = this->array[i] * multiplier;

    return vector;
}

MVector MVector::operator*(const int multiplier)
{
    return *this * static_cast<double>(multiplier);
}

MVector MVector::operator/(const double denominator)
{
    MVector vector;

    for (int i=0; i<size; i++)
        vector[i] = this->array[i] / denominator;

    return vector;
}

MVector MVector::operator/(const int denominator)
{
    return *this / static_cast<double>(denominator);
}

MVector MVector::operator+(const MVector other)
{
    MVector vector;

    for (int i=0; i<size; i++)
        vector[i] = this->array[i] + other.array[i];

    return vector;
}

double &MVector::operator[](int index)
{
    Q_ASSERT("index out of range!" && index >= 0 && index < size);

    array.at(index);
    return array[index];
}

/*!
 * \brief MVector::zeroes retuns a MVector with zero in each component.
 */
MVector MVector::zeroes()
{
    return MVector();
}

/*!
 * \brief MVector::getRelativeVector returns a MVector based on this relative to baseVector.
 * baseVector is expected to be absolute.
 */
MVector MVector::getRelativeVector(MVector baseVector)
{
    // cp vector data
    MVector deviationVector = *this;

    // calculate deviation / %
    for (int i=0; i<size; i++)
    {
        deviationVector[i] = 100 * ((this->array[i] /  baseVector[i]) - 1.0);
    }

    return deviationVector;
}

/*!
 * \brief MVector::getAbsoluteVector returns an absolute MVector based on this, which is relative to baseVector.
 * baseVector is expected to be absolute.
 */
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
