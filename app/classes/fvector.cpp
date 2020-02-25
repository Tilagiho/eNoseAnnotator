#include "fvector.h"

#include <QtCore>

/*!
  \class FVector
  \brief The FVector class represents the vector of the average of each functionalistation for one measurement.
  \ingroup classes
  FVector can contain absolute resistance values, but also deviations relative to a base resistance (R0).
*/
FVector::FVector(int n_funcs):
    size(n_funcs)
{
    // init array
    for (int i=0; i<size; i++)
        vector.push_back(0.0);
}

FVector::~FVector()
{

}

QString FVector::toString()
{
    QStringList stringList;

    for (int i=0; i<size; i++)
        stringList << QString::number(vector[i]);

    stringList << userAnnotation.toString() << detectedAnnotation.toString();

    return stringList.join(";");
}

bool FVector::operator==(const FVector &other) const
{
    double epsilon = 0.0001;

    // this == other, if ||this->array[i]-other.array[i]|| < epsilon for i<size
    for (int i=0; i<size; i++)
        if (qAbs(this->vector[i]-other.vector[i]) > epsilon)
            return false;

    return true;
}

bool FVector::operator!=(const FVector &other) const
{
    // not equal
    return !this->operator==(other);
}

FVector FVector::operator*(const double multiplier)
{
    FVector vector;

    for (int i=0; i<size; i++)
        vector[i] = this->vector[i] * multiplier;

    return vector;
}

FVector FVector::operator*(const int multiplier)
{
    return *this * static_cast<double>(multiplier);
}

FVector FVector::operator/(const double denominator)
{
    FVector vector;

    for (int i=0; i<size; i++)
        vector[i] = this->vector[i] / denominator;

    return vector;
}

FVector FVector::operator/(const int denominator)
{
    return *this / static_cast<double>(denominator);
}

FVector FVector::operator+(const FVector other)
{
    FVector vector;

    for (int i=0; i<size; i++)
        vector[i] = this->vector[i] + other.vector[i];

    return vector;
}

double &FVector::operator[](int index)
{
    Q_ASSERT("index out of range!" && index >= 0 && index < size);

    vector.at(index);
    return vector[index];
}

/*!
 returns a MVector with zero in each component.
 */
FVector FVector::zeroes()
{
    return FVector();
}

/*!
   returns a MVector based on this relative to baseVector.
   baseVector is expected to be absolute.
 */
FVector FVector::getRelativeVector(FVector baseVector)
{
    // cp vector data
    FVector deviationVector = *this;

    // calculate deviation / %
    for (int i=0; i<size; i++)
    {
        deviationVector[i] = 100 * ((this->vector[i] /  baseVector[i]) - 1.0);
    }

    return deviationVector;
}

/*!
   returns an absolute MVector based on this, which is relative to baseVector.
   baseVector is expected to be absolute.
 */
FVector FVector::getAbsoluteVector(FVector baseVector)
{
    // cp vector data
    FVector absoluteVector = *this;

    // calculate absolute resistances / Ohm
    for (int i=0; i<FVector::size; i++)
    {
        absoluteVector[i] = ((this->vector[i] / 100.0) + 1.0) * baseVector[i];
    }
    return absoluteVector;
}
