#ifndef MVECTOR_H
#define MVECTOR_H

#include <QtCore>
#include <array>

#include "aclass.h"
#include "annotation.h"
#include "classifier_definitions.h"
#include "functionalisation.h"

// forward declarations
class AbsoluteMVector;
class RelativeMVector;

class MVector
{

public:
    MVector(const MVector &other);
//    MVector(const AbsoluteMVector &other);
//    MVector(const RelativeMVector &other);
    MVector(AbsoluteMVector* baseVector=nullptr, size_t size=nChannels);
    ~MVector();

    static size_t nChannels;

    QString toString();

    bool operator ==(const MVector &other) const;
    bool operator !=(const MVector &other) const;

    MVector operator *(const double denominator);
    MVector operator *(const int denominator);
    MVector operator /(const double denominator);
    MVector operator /(const int denominator);
    MVector operator +(const MVector other);
    MVector operator +(const double value);
    MVector operator -(const MVector other);

    MVector& operator+=(const MVector& other);

    double &operator[] (int index);

    double operator[] (int index) const;

    bool isZeroVector() const;

    double average(const std::vector<bool> &sensorFailures) const;

    /*
     * class annotated by the user
     * -> can be used as base truth
     */
    Annotation userAnnotation;

    /*
     * automatically detected class
     */
    Annotation detectedAnnotation;

//    virtual RelativeMVector getFuncAverageVector(std::vector<int> functionalisation, std::vector<bool> sensorFailures);

//    virtual RelativeMVector getFuncMedianAverageVector(std::vector<int> functionalisation, std::vector<bool> sensorFailures, int nMedian = 4);

//    RelativeMVector getFuncVector(std::vector<int> functionalisation, std::vector<bool> sensorFailures, InputFunctionType inputFunction);

    QMap<QString, double> sensorAttributes;   // map: attributeName, attributeValue

    std::vector<double> getVector() const;

    size_t getSize() const;

    void copyMetaData(const MVector &other);

    bool isZeroVector();

    void setBaseVector(AbsoluteMVector *value);

    MVector getFuncAverageVector(const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures);

    MVector getFuncMedianAverageVector(const Functionalisation & functionalisation, const std::vector<bool> &sensorFailures, int nMedian = 4);

    MVector getFuncVector(const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures, InputFunctionType inputFunction = InputFunctionType::medianAverage);

    AbsoluteMVector *getBaseVector() const;

    MVector squared() const;

    MVector squareRoot() const;

protected:
    size_t size;    // number of sensor values
    std::vector<double> vector;
    AbsoluteMVector* baseVector = nullptr;
};

class AbsoluteMVector : public MVector
{
public:
    AbsoluteMVector(AbsoluteMVector* baseVector=nullptr, size_t size=nChannels);
    AbsoluteMVector(const MVector &other);

    ~AbsoluteMVector();

    /*
     * returns the deviation vector (/ %) of this relative to baseVector
     */
    RelativeMVector getRelativeVector() const;
};

class RelativeMVector : public MVector
{
public:
    RelativeMVector(AbsoluteMVector* baseVector=nullptr, size_t size=nChannels);
    RelativeMVector(const MVector &other);
    ~RelativeMVector();

    /*
     * returns the absolute vector (/ Ohm) of this based on baseVector
     */
    AbsoluteMVector getAbsoluteVector() const;
};

Q_DECLARE_METATYPE(MVector);

#endif // MVECTOR_H
