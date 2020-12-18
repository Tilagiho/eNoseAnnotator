#include "mvector.h"
#include "measurementdata.h"

#include "../widgets/linegraphwidget.h"

#include <QtCore>

#include <stdexcept>

size_t MVector::nChannels = 64;

MVector::MVector(const MVector &other)
{
    this->vector = other.getVector();
    this->size = other.getSize();

    copyMetaData(other);
}

//MVector::MVector(const AbsoluteMVector &other):
//    MVector(static_cast<MVector>(other))
//{}

//MVector::MVector(const RelativeMVector &other):
//    MVector(static_cast<MVector>(other))
//{}

/*!
  \class MVector
  \brief The MVector class represents one measurement vector of the eNose sensor.
  \ingroup classes
  The size measurement values received every two seconds from the eNose sensor can be stored in MVectors.
  MVector can contain absolute resistance values, but also deviations relative to a base resistance (R0).
*/
MVector::MVector(AbsoluteMVector* baseVector, size_t size):
    baseVector(baseVector),
    size(size),
    vector(size, 0.0)   // init array
{
}

MVector::~MVector()
{

}

void MVector::setBaseVector(AbsoluteMVector *value)
{
    baseVector = value;
}

/*!
 * \brief MVector::toString returns QString with format "ch0;ch1;...;ch63;userAnnoation;detectedAnnotation"
 * \return
 */
QString MVector::toString()
{
    QStringList stringList;

    for (int i=0; i<size; i++)
        stringList << QString::number(vector[i]);

    stringList << userAnnotation.toString();
    stringList << detectedAnnotation.toString();

    return stringList.join(";");
}

bool MVector::operator==(const MVector &other) const
{
    Q_ASSERT(other.size == this->size);

    double epsilon = 0.0001;

    // this == other, if ||this->array[i]-other.array[i]|| < epsilon for i<size
    for (int i=0; i<size; i++)
        if (qAbs(this->vector[i]-other.vector[i]) > epsilon)
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
    MVector vector(baseVector);
    vector.copyMetaData(*this);

    for (int i=0; i<size; i++)
    {
        if (qIsInf(this->vector[i]))
            vector[i] = this->vector[i] * multiplier;
        else    // deal with infinte values
            vector[i] = qInf();
    }

    return vector;
}

MVector MVector::operator*(const int multiplier)
{
    return (*this) * static_cast<double>(multiplier);
}

MVector MVector::operator/(const double denominator)
{
    MVector vector(baseVector);
    vector.copyMetaData(*this);

    for (int i=0; i<size; i++)
    {
        if (qIsFinite(this->vector[i]))
            vector[i] = this->vector[i] / denominator;
        else    // deal with infinite values
            vector[i] = qInf();
    }

    return vector;
}

MVector MVector::operator/(const int denominator)
{
    return *this / static_cast<double>(denominator);
}

MVector MVector::operator+(const MVector other)
{
    Q_ASSERT(other.size == this->size);

    MVector vector(baseVector);
    vector.copyMetaData(*this);

    for (int i=0; i<size; i++)
    {
        if (qIsFinite(this->vector[i]) && qIsFinite(other.vector[i]))
            vector[i] = this->vector[i] + other.vector[i];
        else    // deal with infinte values
            vector[i] = qInf();
    }

    return vector;
}

MVector MVector::operator +(const double value)
{
    MVector vector(baseVector);
    vector.copyMetaData(*this);

    for (int i=0; i<size; i++)
    {
        if (qIsFinite(this->vector[i]))
            vector[i] = this->vector[i] + value;
        else    // deal with infinte values
            vector[i] = qInf();
    }

    return vector;
}


MVector MVector::operator-(const MVector other)
{
    Q_ASSERT(other.size == this->size);

    MVector vector(baseVector);
    vector.copyMetaData(*this);

    for (int i=0; i<size; i++)
    {
        if (qIsFinite(this->vector[i]) && qIsFinite(other.vector[i]))
            vector[i] = this->vector[i] - other.vector[i];
        else    // deal with infinte values
            vector[i] = qInf();
    }

    return vector;
}

MVector& MVector::operator+=(const MVector& other)
{
    auto otherVec = other.getVector();
    for (size_t i=0; i<size; i++)
        vector[i] += otherVec[i];

    return *this;
}


double &MVector::operator[](int index)
{
    Q_ASSERT("index out of range!" && index >= 0 && index < size);

    return vector[index];
}

double MVector::operator[](int index) const
{
    Q_ASSERT("index out of range!" && index >= 0 && index < size);

    return vector[index];
}

bool MVector::isZeroVector() const
{
    for (size_t i=0; i<size; i++)
    {
        if (!qFuzzyIsNull(vector[i]))
            return false;
    }

    return  true;
}


double MVector::average(const std::vector<bool> &sensorFailures) const
{
    double sum = 0.;
    int count = 0;
    for (size_t i=0; i<size; i++)
    {
        if (!sensorFailures[i])
        {
            sum += vector[i];
            count++;
        }
    }

    return sum / count;
}

/*!
 * \brief MVector::getFuncVector return MVector of functionalisation averages
 * \param functionalisation
 * \param sensorFailures
 * \return
 */
MVector MVector::getFuncAverageVector(const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    Q_ASSERT(functionalisation.size() == this->size);
    Q_ASSERT(sensorFailures.size() == this->size);

    // get func map
    auto funcMap = functionalisation.getFuncMap(sensorFailures);

    // no funcs set:
    // return relativevector
    if (funcMap.size() == 1)
        return *this;

    // init func vector
    RelativeMVector funcVector(nullptr, funcMap.size());

    // copy atributes
    funcVector.userAnnotation = userAnnotation;
    funcVector.detectedAnnotation = detectedAnnotation;

    // calc averages of functionalisations
    for (int i=0; i<MVector::nChannels; i++)
    {
        if (!sensorFailures[i])
        {
            int func = functionalisation[i];
            int vectorIndex = funcMap.keys().indexOf(func);
            funcVector[vectorIndex] += vector[i] / funcMap[func];
        }
    }

    return funcVector;
}

MVector MVector::getFuncMedianAverageVector(const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures, int nMedian)
{
    Q_ASSERT(functionalisation.size() == this->size);
    Q_ASSERT(sensorFailures.size() == this->size);

    // get func map
    auto funcMap = functionalisation.getFuncMap(sensorFailures);

    // init func vector
    RelativeMVector medianAverageVector(nullptr, funcMap.size());

    // copy atributes
    medianAverageVector.userAnnotation = userAnnotation;
    medianAverageVector.detectedAnnotation = detectedAnnotation;

    // create list of functionalisation values
    QMap<int, QList<double>> funcValueMap;
    for (int i=0; i<MVector::nChannels; i++)
    {
        if (!sensorFailures[i])
        {
            int func = functionalisation[i];
            funcValueMap[func].append(vector[i]);
        }
    }

    // calculate values of medianAverage Vector
    for (int func : funcValueMap.keys())
    {
        if (funcValueMap[func].size() > nMedian)
        {
            // sort value list
            std::sort(funcValueMap[func].begin(), funcValueMap[func].end());

            // remove non-median values
            // -> nMedian values remain in list
            bool removeLast = false;
            while(funcValueMap[func].size() > nMedian)
            {
                if (removeLast)
                    funcValueMap[func].removeLast();
                else
                    funcValueMap[func].removeFirst();

                removeLast = !removeLast;
            }
        }

        // calculate averages of median values
        int vectorIndex = funcMap.keys().indexOf(func);

        for (int i=0; i<funcValueMap[func].size(); i++)
            medianAverageVector[vectorIndex] += funcValueMap[func][i] / funcValueMap[func].size();
    }

    return medianAverageVector;
}

MVector MVector::getFuncVector(const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures, InputFunctionType inputFunction)
{
    Q_ASSERT(functionalisation.size() == this->size);
    Q_ASSERT(sensorFailures.size() == this->size);

    switch (inputFunction) {
    case InputFunctionType::none:
        return *this;
    case InputFunctionType::average:
        return getFuncAverageVector(functionalisation, sensorFailures);
    case InputFunctionType::medianAverage:
        return getFuncMedianAverageVector(functionalisation, sensorFailures);
    default:
        throw std::invalid_argument("Unhandled InputFunctionType!");
    }
}

AbsoluteMVector *MVector::getBaseVector() const
{
    return baseVector;
}

MVector MVector::squared() const
{
    MVector squaredVector(baseVector);

    for (size_t i=0; i<size; i++)
        squaredVector[i] = qPow(vector[i], 2);

    return squaredVector;
}

MVector MVector::squareRoot() const
{
    MVector squareRootVector(baseVector);

    for (size_t i=0; i<size; i++)
        squareRootVector[i] = qPow(vector[i], 0.5);

    return squareRootVector;
}

std::vector<double> MVector::getVector() const
{
    return vector;
}

size_t MVector::getSize() const
{
    return size;
}

void MVector::copyMetaData(const MVector &other)
{
    this->baseVector = other.getBaseVector();
    this->sensorAttributes = other.sensorAttributes;

    this->userAnnotation = other.userAnnotation;
    this->detectedAnnotation = other.detectedAnnotation;
}

bool MVector::isZeroVector()
{
    for (size_t i=0; i<size; i++)
    {
        if (!qFuzzyIsNull(vector[i]))
            return false;
    }
    return true;
}

AbsoluteMVector::AbsoluteMVector(AbsoluteMVector* baseVector, size_t size):
    MVector(baseVector, size)
{
    Q_ASSERT(baseVector == nullptr || baseVector->getSize() == size);
}

AbsoluteMVector::AbsoluteMVector(const MVector &other):
    MVector(other)
{

}

/*!
   returns a MVector based on this relative to baseVector.
   baseVector is expected to be absolute.
 */
RelativeMVector AbsoluteMVector::getRelativeVector() const
{
    // no baseVector set:
    // return zero vector
    if (baseVector == nullptr)
        return RelativeMVector(nullptr, size);

    // cp vector data
    RelativeMVector relativeVector(baseVector, size);
    relativeVector.copyMetaData(*this);

    // calculate deviation / %
    for (int i=0; i<size; i++)
    {
        // normal case: finite values
        if (qIsFinite((*baseVector)[i]) && qIsFinite(this->vector[i]))
            relativeVector[i] = 100 * ((this->vector[i] /  (*baseVector)[i]) - 1.0);
        // deal with infinite values
        else
        {
            if (qIsInf((*baseVector)[i]) && qIsInf(this->vector[i]))   // both infinite
                relativeVector[i] = 0.0;
            else if (qIsInf((*baseVector)[i]))                         // vector finite, baseVector infinite
                relativeVector[i] = 0;
            else                                                    // vector infinite, baseVector finite
                relativeVector[i] = qInf();
        }
    }

    return relativeVector;
}

RelativeMVector::RelativeMVector(AbsoluteMVector* baseVector, size_t size):
    MVector(baseVector, size)
{
    Q_ASSERT(baseVector == nullptr || baseVector->getSize() == size);
}

RelativeMVector::RelativeMVector(const MVector &other):
    MVector(other)
{

}

RelativeMVector::~RelativeMVector()
{}

/*!
   returns an absolute MVector based on this, which is relative to baseVector.
   baseVector is expected to be absolute.
 */
AbsoluteMVector RelativeMVector::getAbsoluteVector() const
{
    // no baseVector set:
    // return zero vector
    if (baseVector == nullptr)
        return AbsoluteMVector(nullptr, size);

    // cp vector data
    AbsoluteMVector absoluteVector(baseVector, size);
    absoluteVector.copyMetaData(*this);

    // calculate absolute resistances / Ohm
    for (int i=0; i<size; i++)
    {
        absoluteVector[i] = ((this->vector[i] / 100.0) + 1.0) * (*baseVector)[i];
    }
    return absoluteVector;
}

AbsoluteMVector::~AbsoluteMVector()
{}
