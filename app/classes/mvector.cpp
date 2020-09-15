#include "mvector.h"
#include "measurementdata.h"

#include "../widgets/linegraphwidget.h"

#include <QtCore>

#include <stdexcept>

int MVector::nChannels = 64;

/*!
  \class MVector
  \brief The MVector class represents one measurement vector of the eNose sensor.
  \ingroup classes
  The size measurement values received every two seconds from the eNose sensor can be stored in MVectors.
  MVector can contain absolute resistance values, but also deviations relative to a base resistance (R0).
*/
MVector::MVector(int size):
    size(size),
    vector(size, 0.0)   // init array
{
}

MVector::~MVector()
{

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
    MVector vector;

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
    return *this * static_cast<double>(multiplier);
}

MVector MVector::operator/(const double denominator)
{
    MVector vector;

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

    MVector vector;

    for (int i=0; i<size; i++)
    {
        if (qIsFinite(this->vector[i]) && qIsFinite(other.vector[i]))
            vector[i] = this->vector[i] + other.vector[i];
        else    // deal with infinte values
            vector[i] = qInf();
    }

    return vector;
}

double &MVector::operator[](int index)
{
    Q_ASSERT("index out of range!" && index >= 0 && index < size);

    return vector[index];
}

/*!
 returns a MVector with zero in each component.
 */
MVector MVector::zeroes()
{
    return MVector();
}

/*!
   returns a MVector based on this relative to baseVector.
   baseVector is expected to be absolute.
 */
MVector MVector::getRelativeVector(MVector baseVector)
{
    Q_ASSERT(baseVector.size == this->size);

    // cp vector data
    MVector deviationVector = *this;

    // calculate deviation / %
    for (int i=0; i<size; i++)
    {
        // normal case: finite values
        if (qIsFinite(baseVector[i]) && qIsFinite(this->vector[i]))
            deviationVector[i] = 100 * ((this->vector[i] /  baseVector[i]) - 1.0);
        // deal with infinite values
        else
        {
            if (qIsInf(baseVector[i]) && qIsInf(this->vector[i]))   // both infinite
                deviationVector[i] = 0.0;
            else if (qIsInf(baseVector[i]))                         // vector finite, baseVector infinite
                deviationVector[i] = this->vector[i] / LineGraphWidget::maxVal;
            else                                                    // vector infinite, baseVector finite
                deviationVector[i] = qInf();
        }
    }

    return deviationVector;
}

/*!
   returns an absolute MVector based on this, which is relative to baseVector.
   baseVector is expected to be absolute.
 */
MVector MVector::getAbsoluteVector(MVector baseVector)
{
    Q_ASSERT(baseVector.size == this->size);

    // cp vector data
    MVector absoluteVector = *this;

    // calculate absolute resistances / Ohm
    for (int i=0; i<size; i++)
    {
        absoluteVector[i] = ((this->vector[i] / 100.0) + 1.0) * baseVector[i];
    }
    return absoluteVector;
}

/*!
 * \brief MVector::getFuncVector return MVector of functionalisation averages
 * \param functionalisation
 * \param sensorFailures
 * \return
 */
MVector MVector::getAverageFuncVector(std::vector<int> functionalisation, std::vector<bool> sensorFailures)
{
    Q_ASSERT(functionalisation.size() == this->size);
    Q_ASSERT(sensorFailures.size() == this->size);

    // get func map
    auto funcMap = MeasurementData::getFuncMap(functionalisation, sensorFailures);

    // init func vector
    MVector funcVector(funcMap.size());

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

MVector MVector::getMedianAverageFuncVector(std::vector<int> functionalisation, std::vector<bool> sensorFailures, int nMedian)
{
    Q_ASSERT(functionalisation.size() == this->size);
    Q_ASSERT(sensorFailures.size() == this->size);

    // get func map
    auto funcMap = MeasurementData::getFuncMap(functionalisation, sensorFailures);

    // init func vector
    MVector medianAverageVector(funcMap.size());

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

MVector MVector::getFuncVector(std::vector<int> functionalisation, std::vector<bool> sensorFailures, InputFunctionType inputFunction)
{
    Q_ASSERT(functionalisation.size() == this->size);
    Q_ASSERT(sensorFailures.size() == this->size);

    switch (inputFunction) {
    case InputFunctionType::none:
        return *this;
    case InputFunctionType::average:
        return getAverageFuncVector(functionalisation, sensorFailures);
    case InputFunctionType::medianAverage:
        return getMedianAverageFuncVector(functionalisation, sensorFailures);
    default:
        throw std::invalid_argument("Unhandled InputFunctionType!");
    }
}

std::vector<double> MVector::getVector() const
{
    return vector;
}
