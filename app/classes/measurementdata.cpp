#include "measurementdata.h"

#include <QDateTime>
#include <QVector>
#include <QtMath>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

#include "aclass.h"

/*!
 * \class MeasurementData
 * \brief Container for all data concerning the current measurement.
 * Contains all vectors of the current measurement, its meta info & the current selection.
 * Provides functionality for loading & saving measurements & selections.
 *
 */

QString MeasurementData::funcName = "None";
std::vector<int> MeasurementData::functionalisation(MVector::nChannels, 0);
std::vector<bool> MeasurementData::sensorFailures(MVector::nChannels, 0);

MeasurementData::MeasurementData(QObject *parent, int nChannels) :
    QObject(parent)
{
    // zero init info
    setComment("");
    setSensorId("");

    // init class list
    classList = aClass::staticClassSet.toList();
}

MeasurementData::~MeasurementData()
{
    clear();
}

/*!
 * \brief MeasurementData::getRelativeData returns a map of the vectors contained in the MeasurementData converted into relative vectors
 * \return
 */
const QMap<uint, MVector> MeasurementData::getRelativeData()
{
    QMap<uint, MVector> relativeData;

    for (uint timestamp : data.keys())
        relativeData[timestamp] = data[timestamp].getRelativeVector(getBaseLevel(timestamp));

    return relativeData;
}

const QMap<uint, MVector> MeasurementData::getFuncData()
{
    // only one func set
    // -> return full relative data
    if (getFuncMap().size() == 1)
        return getRelativeData();

    QMap<uint, MVector> relativeData = getRelativeData();
    QMap<uint, MVector> funcData;
    for (int timestamp : relativeData.keys())
        funcData[timestamp] = relativeData[timestamp].getFuncVector(functionalisation, sensorFailures);

    return funcData;
}

/*!
 * \brief MeasurementData::getAbsoluteData returns a map of the vectors contained in the MeasurementData as absolute vectors
 * \return
 */
const QMap<uint, MVector> MeasurementData::getAbsoluteData()
{
    return data;
}

/*!
 * \brief MeasurementData::getSelectionMap returns map of the vectors in the current selection
 * \return
 */
const QMap<uint, MVector> MeasurementData::getSelectionMap()
{
    return selectedData;
}

int MeasurementData::getNFuncs() const
{
    QList<int> funcKeys = getFuncMap().keys();
    double maxFunc = *std::max_element(funcKeys.begin(), funcKeys.end());
    return maxFunc + 1;
}

/*!
 * \brief MeasurementData::clear deletes all data from MeasurementData
 */
void MeasurementData::clear()
{
    clearSelection();

    baseLevelMap.clear();
    data.clear();


    std::vector<bool> zeroFailures;
    for (int i=0; i<MVector::nChannels; i++)
        zeroFailures.push_back(false);
    setSensorFailures(zeroFailures);

    setComment("");
    setSensorId("");

    // take filename away from saveFilename so the directory stays
    if (saveFilename.endsWith(".csv"))
    {
        QStringList pathList = saveFilename.split("/");
        if (!pathList.isEmpty())
            pathList.removeLast();
        setSaveFilename(pathList.join("/") + "/");
    }

    setDataChanged(false);
}

/*!
 * \brief MeasurementData::clearSelection deletes the current selection
 */
void MeasurementData::clearSelection()
{
    selectedData.clear();
    emit selectionCleared();
}

/*!
 * \brief MeasurementData::addMeasurement adds \a vector at \a timestamp
 * if the vector map was previously empty, set  \a vector as start timestamp of the measurement
 * \param timestamp
 * \param vector
 */
void MeasurementData::addMeasurement(uint timestamp, MVector vector)
{
    // sync sensor attributes
    for (QString attributeName : vector.sensorAttributes.keys())
        if (!sensorAttributes.contains(attributeName))
            addAttributes(QSet<QString>{attributeName});

    for (QString attributeName : sensorAttributes)
        if (!vector.sensorAttributes.keys().contains(attributeName))
            vector.sensorAttributes[attributeName] = 0.0;

    //  double usage of timestamps:
    if (data.contains(timestamp))
    {
        qWarning() << timestamp << ": double usage of timestamp!";
        // case 1: last timestamp was not skipped
        // -> increment current timestamp
        if (data.contains(timestamp-1))
            timestamp++;
        // case 2: last timesstamp was skipped:
        // move last measurement to skipped timestamp
        else
            data[timestamp-1] = data[timestamp];
    }

    // empty data:
    // this timestamp is startTimestamp of data
    if (data.isEmpty())
    {
        emit startTimestempSet(timestamp);
    }

    // calculate deviation in %
    MVector deviationVector;
    MVector baseLevelVector = getBaseLevel(timestamp);

    deviationVector = vector.getRelativeVector(baseLevelVector);

    // add data, update dataChanged
    data[timestamp] = vector;
    if (!dataChanged)
        setDataChanged(true);

    emit dataAdded(deviationVector, timestamp, true);
    emit absoluteDataAdded(vector, timestamp, true);
}

/*!
 * \brief MeasurementData::addMeasurement adds \a vector at \a timestamp
 * \param timestamp
 * \param vector
 * \param baseLevel
 */
void MeasurementData::addMeasurement(uint timestamp, MVector vector, MVector baseLevel)
{
    // if new baseLevel: add to baseLevelMap
    if (baseLevelMap.isEmpty() || baseLevel != getBaseLevel(timestamp))
        baseLevelMap[timestamp] = baseLevel;

    // add vector to data
    addMeasurement(timestamp, vector);
}

void MeasurementData::setData(QMap<uint, MVector> absoluteData, QMap<uint, MVector> baseVectors)
{
    // clear data
    data.clear();
    baseLevelMap.clear();

    // set baseVectors
    baseLevelMap = baseVectors;

    // add vectors
    emit setReplotStatus(false);
    for (uint timestamp : absoluteData.keys())
        addMeasurement(timestamp, absoluteData[timestamp]);

    emit setReplotStatus(true);
}

void MeasurementData::setClasslist(QList<aClass> newClassList)
{
    classList.clear();
    aClass::staticClassSet.clear();

    for (aClass aclass : newClassList)
        addClass(aclass);
}

void MeasurementData::setSensorAttributes(QSet<QString> newSensorAttributes)
{
    sensorAttributes.clear();

    addAttributes(newSensorAttributes);
}

void MeasurementData::setSaveFilename(QString newSaveFilename)
{
    if (newSaveFilename != saveFilename)
    {
        saveFilename = newSaveFilename;
        emit saveFilenameSet();
    }
}

MVector MeasurementData::getMeasurement(uint timestamp)
{
    Q_ASSERT(data.contains(timestamp));

    return data[timestamp];
}

QString MeasurementData::getComment()
{
    return dataComment;
}

void MeasurementData::setComment(QString new_comment)
{
    if (dataComment != new_comment)
    {
        dataComment = new_comment;
        setDataChanged(true);
        emit commentSet(dataComment);
    }
}

void MeasurementData::setFailures(const std::vector<bool> failures)
{
    Q_ASSERT(failures.size() == MVector::nChannels);

    if (failures != sensorFailures)
    {
        sensorFailures = failures;
        setDataChanged(true);
        emit sensorFailuresSet(failures);
    }
}

/*!
 * \brief MeasurementData::setFailures sets sensor failures from a QString.
 * \param failureString is expected to be a numeric string of length MVector::nChannels.
 */
void MeasurementData::setFailures(QString failureString)
{
    Q_ASSERT(failureString.length()==MVector::nChannels);

    std::vector<bool> failures;

    for (int i=0; i<MVector::nChannels; i++)
    {
        Q_ASSERT(failureString[i]=="0" || failureString[i]=="1");
        failures.push_back(failureString[i] == "1");
    }

    setFailures(failures);
}

void MeasurementData::setSensorId(QString newSensorId)
{
    if (newSensorId != sensorId)
    {
        sensorId = newSensorId;

        // changing sensor id of empty measurement data should not trigger dataChanged
        if (!data.isEmpty())
            setDataChanged(true);

        emit sensorIdSet(sensorId);
    }
}

/*!
 * \brief MeasurementData::setBaseLevel adds \a baseLevel to the base level vector map. All vectors added after \a timestamp will be normed to \a baseLevel if converted into a relative vector.
 */
void MeasurementData::setBaseLevel(uint timestamp, MVector baseLevel)
{
    Q_ASSERT (!baseLevelMap.contains(timestamp));

    if (baseLevelMap.isEmpty() || baseLevelMap.last() != baseLevel)
    {
        baseLevelMap.insert(timestamp, baseLevel);
        setDataChanged(true);
//        qDebug() << "New baselevel at " << timestamp << ":\n" << baseLevelMap[timestamp].toString();
    }
}

QList<aClass> MeasurementData::getClassList() const
{
    return classList;
}

std::vector<int> MeasurementData::getFunctionalisation() const
{
    return functionalisation;
}

void MeasurementData::setFunctionalisation(const std::vector<int> &value)
{
    Q_ASSERT(value.size() == MVector::nChannels);

    if (value != functionalisation)
    {
        // set func name
        if (funcName == "None")
        {
            for (uint i=0; i<MVector::nChannels; i++)
                if (functionalisation[i] != 0)
                    setFuncName("Custom");
        }

        // set functionalisation
        functionalisation = value;
        setDataChanged(true);
        emit functionalisationChanged();
    }
}

/*!
 * \brief MeasurementData::getFuncMap return map of functionalsations & the number of their appearences in funcs, ignoring channels with fails[channel] == true
 * \param funcs
 * \param sensorFailures
 * \return
 */
QMap<int, int> MeasurementData::getFuncMap(const std::vector<int> &funcs, std::vector<bool> fails)
{
//    Q_ASSERT(funcs.size() == MVector::nChannels);
//    Q_ASSERT(fails.size() == MVector::nChannels);

    // get number of functionalisations, ignore channels with sensor failures
    QMap<int, int> funcMap;
    for (int i=0; i<MVector::nChannels; i++)
    {
        int func = funcs[i];
        if (!funcMap.contains(func))
            // sensor failure in channel i:
            // assign 0, so failing funcs are not overseen
            funcMap[func] = fails[i] ? 0 : 1;
        else
            if (!fails[i])
                funcMap[func]++;
    }
    return funcMap;
}

QMap<int, int> MeasurementData::getFuncMap()
{
    return getFuncMap(functionalisation, sensorFailures);
}

/*!
 * \brief MeasurementData::changed returns change status of MeasurementData.
 * Is \c true if any data was changed since loading or saving the measurement
 *
 */
bool MeasurementData::isChanged() const
{
    return dataChanged;
}

/*!
 * \brief MeasurementData::setDataChanged should be used when changing dataChanged in order to keep the GUI updated.
 * \param newValue
 * emits dataChangedSet if MeasurementData::dataChanged is changed.
 */
void MeasurementData::setDataChanged(bool newValue)
{
    if (newValue != dataChanged)
    {
        dataChanged = newValue;
        emit dataChangedSet(newValue);
    }
}

/*!
 * \brief MeasurementData::getBaseLevel returns the last base level MVector set before \a timestamp.
 * \param timestamp
 */
MVector MeasurementData::getBaseLevel(uint timestamp)
{
    if (baseLevelMap.isEmpty())
    {
        qWarning() << "No baselevel was set!" << "\n";
        return  MVector::zeroes();
    }

    uint bsTimestamp = 0;

    //  go through baseLevelMap and store ts in bsTimestamp until timestamp < ts
    // -> bsTimestamp is last ts
    // or until timestamp == ts
    // -> bsTimestamp = ts
    for (auto  ts: baseLevelMap.keys())
    {
        if (ts > timestamp)
            break;
        else if (ts == timestamp)
        {
            bsTimestamp = ts;
            break;
        }

        bsTimestamp = ts;
    }
    Q_ASSERT ("Error: No baselevel was set!" && bsTimestamp!=0);

    return baseLevelMap[bsTimestamp];
}

std::vector<bool> MeasurementData::getSensorFailures() const
{
    return sensorFailures;
}

void MeasurementData::setSensorFailures(const std::vector<bool> &value)
{
    Q_ASSERT(value.size() == MVector::nChannels);

    if (value != sensorFailures)
    {
        sensorFailures = value;
        setDataChanged(true);
        emit sensorFailuresSet(value);
    }
}

QString MeasurementData::getSensorId() const
{
    return sensorId;
}

/*!
 * \brief MeasurementData::getFailureString returns a QString of length MVector::nChannels consisting of '0' & '1'.
 * \return
 */
QString MeasurementData::getFailureString()
{
    QString failureString("");

    for (int i=0; i<MVector::nChannels; i++)
        if (sensorFailures[i])
            failureString += "1";
        else
            failureString += "0";

    return failureString;
}

bool MeasurementData::contains(uint timestamp)
{
    return data.contains(timestamp);
}

bool MeasurementData::saveData(QString filename)
{
    Q_ASSERT(!data.isEmpty());

    return saveData(filename, data);
}

/*!
 * \brief MeasurementData::saveData saves all vectors in \a map & meta info in \a filename.
 */
bool MeasurementData::saveData(QString filename, QMap<uint, MVector> map)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        throw std::runtime_error("Unable to open file: file.errorString()");

    QTextStream out(&file);
    // write info
    // version
    out << "#measurement data v" << savefileFormatVersion << "\n";

    // sensorId
    out << "#sensorId:" << sensorId << "\n";

    // sensor failures
    out << "#failures:" << getFailureString() << "\n";
    if (!dataComment.isEmpty())
    {
        // go through dataComment line-by-line
        QTextStream commentStream(&dataComment);
        QString line;

        while (commentStream.readLineInto(&line))
            out << "#" << line << "\n";
    }

    // sensor functionalisation
    out << "#funcName:" << funcName << "\n";
    QStringList funcList;
    for (int i=0; i<functionalisation.size(); i++)
        funcList << QString::number(functionalisation[i]);
    out << "#functionalisation:" << funcList.join(";") << "\n";

    // base vector
    for (uint timestamp : baseLevelMap.keys())
    {
        out << "#baseLevel:" << getTimestampStringFromUInt(timestamp) << ";";
        QStringList valueList;
        for (int i=0; i<MVector::nChannels; i++)
            valueList << QString::number(baseLevelMap[timestamp][i], 'g', 10);
        out <<  valueList.join(";") << "\n";
    }

    // classes
    out << "#classes:";
    QStringList classStringList;
    for (aClass c : classList)
        classStringList << c.toString();
    out << classStringList.join(";") << "\n";

    // write header
    QStringList headerList;

    headerList << "#header:timestamp";

    for (int i=0; i<MVector::nChannels; i++)
        headerList << "ch" + QString::number(i+1);

    for (QString sensorAttribute : sensorAttributes)
        headerList << sensorAttribute;

    headerList << "user defined class";
    headerList << "detected class";

    out << headerList.join(";") << "\n";

    // write data
    auto iter = map.begin();

    while (iter != map.end())
    {
        QStringList valueList;
        valueList << getTimestampStringFromUInt(iter.key());      // timestamp

        // vector
        for (int i=0; i<MVector::nChannels; i++)
            valueList << QString::number(iter.value()[i], 'g', 10);
        // sensor attributes
        for (QString attribute : iter.value().sensorAttributes.keys())
            valueList << QString::number(iter.value().sensorAttributes[attribute], 'g', 10);

        // classes
        valueList << iter.value().userAnnotation.toString() << iter.value().detectedAnnotation.toString();
        out <<  valueList.join(";") << "\n";

        iter++;
    }
    setDataChanged(false);
    return true;
}

/*!
 * \brief MeasurementData::saveSelection  saves current selection in \a filename.
 */
bool MeasurementData::saveSelection(QString filename)
{
    Q_ASSERT("Selection data is empty!" && !selectedData.isEmpty());
    return saveData(filename, selectedData);
}

/*!
 * \brief MeasurementData::saveAverageSelectionMeasVector saves average vector of the current selection.
 * \param widget
 * \param filename
 * \return
 */
bool MeasurementData::saveAverageSelectionMeasVector(QString filename)
{
    Q_ASSERT("Selection data is empty!" && !selectedData.isEmpty());

    // calculate average selection vector
    MVector selectionMeasVector = getSelectionVector();

    // save average vector in file
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly))
        throw std::runtime_error("Unable to open file: file.errorString()");

    QTextStream out(&file);

    // write to file:
    // vector string without annotations
    QStringList vectorList = selectionMeasVector.toString().split(";");
    vectorList = vectorList.mid(0, vectorList.size()-2);

    int i = 0;
    for (QString valueString : vectorList)
    {
        out << QString::number(i) << ";" << valueString << "\n";
        i++;
    }
    return true;
}

/*!
 * \brief MeasurementData::saveAverageSelectionFuncVector saves average vector of the current selection.
 * \param widget
 * \param filename
 * \return
 */
bool MeasurementData::saveAverageSelectionFuncVector(QString filename)
{
    Q_ASSERT("Selection data is empty!" && !selectedData.isEmpty());

    // calculate average selection vector
    MVector selectionVector = getSelectionVector();
    MVector selectionFuncVector = selectionVector.getFuncVector(functionalisation, sensorFailures);

    // save average vector in file
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly))
        throw std::runtime_error("Unable to open file: file.errorString()");

    QTextStream out(&file);

    // write to file:
    // vector string without annotations
    QStringList vectorList = selectionFuncVector.toString().split(";");
    vectorList = vectorList.mid(0, vectorList.size()-2);

    int i = 0;
    for (QString valueString : vectorList)
    {
        out << QString::number(i) << ";" << valueString << "\n";
        i++;
    }

    return true;
}

void MeasurementData::copyFrom(MeasurementData *otherMData)
{
    // clear local data
    clear();

    // meta data
    setClasslist(otherMData->getClassList());
    setSensorAttributes(otherMData->getSensorAttributes());
    setSaveFilename(otherMData->getSaveFilename());
    setComment(otherMData->getComment());
    setSensorId(otherMData->sensorId);
    setSensorFailures(otherMData->getSensorFailures());
    setFuncName(otherMData->funcName);

    // data
    setData(otherMData->getAbsoluteData(), otherMData->getBaseLevelMap());
    setDataChanged(false);
}

void MeasurementData::setSelection(int lower, int upper)
{
    // ignore existing selections
    if (!selectedData.isEmpty() && selectedData.firstKey() == lower && selectedData.lastKey() == upper)
        return;

    // clear selectedData
    clearSelection();

    // selection deselected
    if (upper < lower)
        return;

    MVector vector;

    qDebug() << "Selection requested: " << lower << ", " << upper;

    // single selection:
    if (lower == upper)
    {
        // update selectData
        selectedData[lower] = data[lower];
    }
    // multi selection:
    else
    {
        QMap<uint, MVector>::iterator beginIter, endIter;
        bool beginIterSet = false, endIterSet = false;

        // find lowest beginTimestamp >= lower & endTimestamp >= higher respectively
        for (uint timestamp : data.keys())
        {
            if (!beginIterSet && timestamp >= lower)
            {
                 beginIter = data.find(timestamp);
                beginIterSet = true;
            }
            if (!endIterSet && timestamp > upper)
            {
                endIter = data.find((timestamp));
                endIterSet = true;
                // increment endIter in order to point one element further than upper
//                endIter++;
                break;
            }
        }

        // upper > timestamp of last vector
        if (!endIterSet)
            endIter = data.end();

        // no vector in the interval [lower; upper]
        if (!beginIterSet || (beginIter != data.end() && beginIter.key()>upper) || (endIter != data.end() && endIter.key()<lower))
            return;

        // add selected vectors to seletedData
        for (auto iter=beginIter; iter != endIter; iter++)
            selectedData[iter.key()] = iter.value();
    }

    // calculate average vector
    if (selectedData.isEmpty())
        return;

    vector = getSelectionVector();

    qDebug() << "Selection made: " << selectedData.firstKey() << ", " << selectedData.lastKey() << "\n" << vector.toString() << "\n";

    emit selectionVectorChanged(vector, sensorFailures, functionalisation);
}

const MVector MeasurementData::getSelectionVector(MultiMode mode)
{
    // only average supported
    Q_ASSERT(mode == MultiMode::Average);

    MVector selectionVector;
    // zero init
    for (int i=0; i<MVector::nChannels; i++)
        selectionVector[i] = 0.0;

    for (auto timestamp : selectedData.keys())
    {
        // ignore zero vectors
        if (selectedData[timestamp] == MVector::zeroes())
            continue;

        // get relative selection vector
        MVector vector = selectedData[timestamp].getRelativeVector(getBaseLevel(timestamp));

        // calculate average
        if (mode == MultiMode::Average)
            for (int i=0; i<MVector::nChannels; i++)
                selectionVector[i] += vector[i] / selectedData.size();
    }

    return selectionVector;
}

QString MeasurementData::sensorFailureString(std::vector<bool> failureBits)
{
    Q_ASSERT(failureBits.size() == MVector::nChannels);

    QString failureString;

    for (int hex=0; hex<MVector::nChannels/4; hex++)
    {
        QString hexadecimal;
        int failureInt = 0;

        for (int bit=0; bit<4; bit++)
            if (failureBits[4*hex+bit])
                failureInt |= 1UL << bit;
        hexadecimal.setNum(failureInt,16);

        failureString = hexadecimal + failureString;
    }
    return failureString;
}

std::vector<bool> MeasurementData::sensorFailureArray(QString failureString)
{
    std::vector<bool> failureArray;
    for (int i=0; i<MVector::nChannels; i++)
        failureArray.push_back(false);

    if (failureString == "None")
        return failureArray;

    for (int hex=0; hex < failureString.length(); hex++)
    {
        int failureInt = QString(failureString[failureString.length()-hex-1]).toInt(nullptr, 16);

        for (int bit=0; bit<4; bit++)
        {
            failureArray[4*hex+bit] = failureInt & (1UL << bit);
        }
    }

    return failureArray;
}

void MeasurementData::setUserAnnotation(Annotation annotation, uint timestamp)
{
    data[timestamp].userAnnotation = annotation;
    selectedData[timestamp].userAnnotation = annotation;

    setDataChanged(true);
    QMap<uint, MVector> changedMap;
    changedMap[timestamp] = data[timestamp];
    emit labelsUpdated(changedMap);
}

void MeasurementData::setUserAnnotationOfSelection(Annotation annotation)
{
    for (auto timestamp : selectedData.keys())
    {
        // add to data and selectedData
        data[timestamp].userAnnotation = annotation;
        selectedData[timestamp].userAnnotation = annotation;
    }

    setDataChanged(true);
    emit labelsUpdated(selectedData);
}

void MeasurementData::setDetectedAnnotation(Annotation annotation, uint timestamp)
{
    data[timestamp].detectedAnnotation = annotation;
    selectedData[timestamp].detectedAnnotation = annotation;

    setDataChanged(true);
    QMap<uint, MVector> changedMap;
    changedMap[timestamp] = data[timestamp];
    emit labelsUpdated(changedMap);
}

void MeasurementData::setDetectedAnnotationOfSelection(Annotation annotation)
{
    for (auto timestamp : selectedData.keys())
    {
        // data and selectedData
        data[timestamp].detectedAnnotation = annotation;
        selectedData[timestamp].detectedAnnotation = annotation;
    }

    setDataChanged(true);
    emit labelsUpdated(selectedData);
}

void MeasurementData::addClass(aClass newClass)
{
    // classes in the class list are always numeric only
    Q_ASSERT("New classes added should be of type CLASS_ONLY!" && newClass.getType() == aClass::Type::CLASS_ONLY);
    Q_ASSERT("Trying to add class name that already exists!" && !aClass::staticClassSet.contains(newClass));

    // add to static aClass map
    aClass::staticClassSet << newClass;

    // add internally
    classList << newClass;

    setDataChanged(true);
}

void MeasurementData::removeClass(aClass oldClass)
{
    Q_ASSERT("New classes removed should be of type CLASS_ONLY!" && oldClass.getType() == aClass::Type::CLASS_ONLY);
    Q_ASSERT("Trying to remove class that does not exists!" && aClass::staticClassSet.contains(oldClass));
    aClass::staticClassSet.remove(oldClass);

    // update measurement data
    QMap<uint, MVector> updatedVectors;

    for (uint timestamp: data.keys())
    {
        bool updated = false;   // flag for updated class

        if (data[timestamp].userAnnotation.contains(oldClass))
        {
            data[timestamp].userAnnotation.remove(oldClass);
            updated = true;
        }
        if (data[timestamp].detectedAnnotation.contains(oldClass))
        {
            data[timestamp].detectedAnnotation.remove(oldClass);
            updated = true;
        }

//        if (data[timestamp].userAnnotation == oldClass)
//        {
//            double value = (userAnnotationType == aClass::Type::NUMERIC) ? 1.0 : -1.0;
//            data[timestamp].userAnnotation = aClass{"", "", value};
//            updated = true;
//        }
//        if (data[timestamp].detectedAnnotation == oldClass)
//        {
//            double value = (detectedAnnotationType == aClass::Type::NUMERIC) ? 1.0 : -1.0;
//            data[timestamp].detectedAnnotation = aClass{"", "", value};
//            updated = true;
//        }

        if (updated)
            updatedVectors[timestamp] = data[timestamp];
    }

    setDataChanged(true);
    emit labelsUpdated(updatedVectors);
}

void MeasurementData::changeClass(aClass oldClass, aClass newClass)
{
    Q_ASSERT("oldClass should be of type CLASS_ONLY!" && oldClass.getType() == aClass::Type::CLASS_ONLY);
    Q_ASSERT("newClass should be of type CLASS_ONLY!" && newClass.getType() == aClass::Type::CLASS_ONLY);

    Q_ASSERT("Trying to change class that does not exist!" && aClass::staticClassSet.contains(oldClass));
    Q_ASSERT("Trying to change to class that already exists!" && !aClass::staticClassSet.contains(newClass));

    // change class in static class map
    aClass::staticClassSet.remove(oldClass);
    aClass::staticClassSet << newClass;

    // remember updated vectors
    QMap<uint, MVector> updatedVectors;

    for (uint timestamp: data.keys())
    {
        bool updated = false;   // flag for updated class

        if (data[timestamp].userAnnotation.contains(oldClass))
        {
            data[timestamp].userAnnotation.changeClass(oldClass, newClass);
            updated = true;
        }
        if (data[timestamp].detectedAnnotation.contains(oldClass))
        {
            data[timestamp].detectedAnnotation.changeClass(oldClass, newClass);
            updated = true;
        }

        if (updated)
            updatedVectors[timestamp] = data[timestamp];
    }

    setDataChanged(true);
    emit labelsUpdated(updatedVectors);
}

QString MeasurementData::getSaveFilename() const
{
    return saveFilename;
}

QString MeasurementData::getTimestampStringFromUInt(uint timestamp)
{
    QDateTime dateTime = QDateTime::fromTime_t(timestamp);
    return dateTime.toString("d.M.yyyy - h:mm:ss");
}

uint MeasurementData::getTimestampUIntfromString(QString string)
{
    QDateTime dateTime = QDateTime::fromString(string, "d.M.yyyy - h:mm:ss");
    return dateTime.toTime_t();
}

void MeasurementData::setFuncName(QString name)
{
    if (name != funcName)
    {
        funcName = name;
        emit funcNameSet(name);
    }
}

void MeasurementData::addAttributes(QSet<QString> attributeNames)
{
    for (QString attributeName : attributeNames)
        Q_ASSERT(!sensorAttributes.contains(attributeName));

    // add attributes to sensorAttributes
    sensorAttributes.unite(attributeNames);

    // add to vectors
    for (MVector vector : data)
        for (QString attributeName : attributeNames)
            vector.sensorAttributes[attributeName] = 0.0;
}

void MeasurementData::deleteAttributes(QSet<QString> attributeNames)
{
    for (QString attributeName : attributeNames)
        Q_ASSERT(sensorAttributes.contains(attributeName));

    // delete attributes from sensorAttributes
    sensorAttributes.intersect(attributeNames);

    // delete from vectors
    for (MVector vector : data)
        for (QString attributeName : attributeNames)
            vector.sensorAttributes.remove(attributeName);
}

void MeasurementData::renameAttribute(QString oldName, QString newName)
{
    Q_ASSERT(sensorAttributes.contains(oldName));
    Q_ASSERT(!sensorAttributes.contains(newName));
    Q_ASSERT(oldName != newName);

    // rename in sensorAttributes
    sensorAttributes.insert(newName);
    sensorAttributes.remove(oldName);

    // rename in vectors
    for (MVector vector : data)
    {
        vector.sensorAttributes[newName] = vector.sensorAttributes[oldName];
        vector.sensorAttributes.remove(oldName);
    }
}

void MeasurementData::resetNChannels()
{
    sensorFailures = std::vector<bool>(MVector::nChannels, false);
    functionalisation = std::vector<int>(MVector::nChannels, 0);
}

QMap<uint, MVector> MeasurementData::getBaseLevelMap() const
{
    return baseLevelMap;
}

QSet<QString> MeasurementData::getSensorAttributes() const
{
    return sensorAttributes;
}

FileReader::FileReader(QString filePath, QObject* parentWidget):
    file(filePath)
{
    Q_ASSERT(!filePath.isEmpty());

    data = new MeasurementData(parentWidget);

    if (!file.open(QIODevice::ReadOnly))
        throw std::runtime_error("Can not open " + filePath.toStdString());

    in.setDevice(&file);
}

FileReader* FileReader::getSpecificReader()
{
    // read first line
    QString line;
    if (!in.readLineInto(&line))
    {
        throw  std::runtime_error(file.fileName().toStdString() + " is empty!");
    }

    if (line.startsWith("#measurement data"))
    {
        return new AnnotatorFileReader(file.fileName());
    }
    else
    {
        return new LeifFileReader(file.fileName());
    }
}

FileReader::FileReaderType FileReader::getType()
{
    return FileReaderType::General;
}

MeasurementData* FileReader::getMeasurementData()
{
    // reset dataChanged
    data->setDataChanged(false);
    return data;
}

FileReader::~FileReader()
{
    if (file.isOpen())
        file.close();

    delete data;
}

AnnotatorFileReader::AnnotatorFileReader(QString filePath):
    FileReader(filePath)
{
}

void AnnotatorFileReader::readFile()
{
    // read data
    QString line;
    while(in.readLineInto(&line))
    {
        lineCount++;
        if (line.startsWith("#"))
            parseHeader(line);
        else
            parseValues(line);
    }
}

FileReader::FileReaderType AnnotatorFileReader::getType()
{
    return FileReaderType::Annotator;
}

void AnnotatorFileReader::parseHeader(QString line)
{
    if (line.startsWith("#measurement data v"))
    {
        formatVersion = line.right(line.length()-QString("#measurement data v").length());
        formatVersion = formatVersion.split(";").join("");
    }
    else if (line.startsWith("#measurement data"))
    {
        formatVersion = "0.1";
    }
    else if (line.startsWith("#sensorId:"))
    {
        QString sensorId = line.right(line.length()-QString("#sensorId:").length());
        sensorId = sensorId.split(";").join("");
        data->setSensorId(sensorId);
    }
    else if (line.startsWith("#failures:"))
    {
        failureString = line.right(line.length()-QString("#failures:").length());
        failureString = failureString.split(";").join("");
    }
    else if (line.startsWith("#funcName:"))
    {
        QString funcName = line.split(":")[1];
        data->setFuncName(funcName);
    }
    else if (line.startsWith("#functionalisation:"))
    {
        QString rawString = line.right(line.length()-QString("#functionalisation:").length());
        QStringList funcList = rawString.split(";");

        for (int i=0; i<funcList.size(); i++)
            functionalistation.push_back(funcList[i].toInt());

        // funcName not stored in file?!
        // -> emit Custom
        if (data->funcName == "None")
        {
            auto functionalisation = data->getFunctionalisation();
            for (uint i=0; i<functionalisation.size(); i++)
            {
                if (functionalisation[i] != 0)
                {
                    data->setFuncName("Custom");
                    break;
                }
            }
        }
    }
    else if (line.startsWith("#baseLevel:"))
    {

        line = line.right(line.length()-QString("#baseLevel:").size());
        QStringList valueList = line.split(";");

        // get timestamp: uint or string
        uint timestamp;
        bool isInt;
        timestamp = valueList[0].toUInt(&isInt);

        if (!isInt)
            timestamp = data->getTimestampUIntfromString(valueList[0]);

        // get base level vector
        MVector baseLevel(valueList.size()-1);
        for (int i=0; i<valueList.size()-1; i++)
            baseLevel[i] = valueList[i+1].toDouble();

        baseLevelMap[timestamp] = baseLevel;
    }
    else if (line.startsWith("#classes:"))
    {
        QStringList classStringList =  line.right(line.length()-QString("#classes:").length()).split(";");
        for (QString classString : classStringList)
        {
            if (classString == "")
                continue;   // ignore empty classStrings

            if (!aClass::isClassString(classString))
                throw std::runtime_error("Error in line " + std::to_string(lineCount+1) + ".\n" + classString.toStdString() + " is not a class string!");
            aClass c = aClass::fromString(classString);

            if (!data->getClassList().contains(c))
                data->addClass(c);
        }
    }
    else if (line.startsWith("#header:"))
    {
        // get indexes
        QRegularExpression rTimestamp ("^timestamp$");
        QRegularExpression rResistance ("^ch(\\d*)$");
        QRegularExpression rUserAnnotation ("^user defined class$");
        QRegularExpression rDetectedAnnotation ("^detected class$");

        QString headerLine = line.split(":")[1];
        QStringList headerItems =  headerLine.split(";");

        for (int i=0; i<headerItems.size(); i++)
        {
            QString headerItem = headerItems[i];

            auto timestampMatch = rTimestamp.match(headerItem);
            auto resistanceMatch = rResistance.match(headerItem);
            auto userAnnoMatch = rUserAnnotation.match(headerItem);
            auto detectedAnnoMatch = rDetectedAnnotation.match(headerItem);

            if (timestampMatch.hasMatch())
                timestampIndex = i;
            else if (resistanceMatch.hasMatch())
                resistanceIndexMap[resistanceMatch.captured(1).toInt()] = i;
            else if (userAnnoMatch.hasMatch())
                userAnnotationIndex = i;
            else if (detectedAnnoMatch.hasMatch())
                detectedAnnotationIndex = i;
            else    // attribute
            {
                sensorAttributeMap[headerItem] = i;
                data->addAttributes(QSet<QString>{headerItem});
            }
        }
        // after parsing the header:
        // reset MVector::nChannels
        emit resetNChannels(resistanceIndexMap.size());

        // set data meta attributes
        data->setFailures(failureString);
        data->setFunctionalisation(functionalistation);

        for (uint timestamp : baseLevelMap.keys())
            data->setBaseLevel(timestamp, baseLevelMap[timestamp]);

    }
    else    // comment
        data->setComment(data->getComment() + line.right(line.length()-1) + "\n");
}

void AnnotatorFileReader::parseValues(QString line)
{
    uint timestamp;
    MVector vector;

    if (line == "") // ignore empty lines
        return;

    // support old format:
    // - measurement values stored as relative vectors

    QStringList query = line.split(";");

    // line normally contains timestamp + vector + sensor attributes + user defined & detected class
    // lines without user defined & detected class are accepted
    int minSize = MVector::nChannels + data->getSensorAttributes().size() + 1;
    if ((query.size() < minSize) || (query.size() > minSize+2))
        throw std::runtime_error("Error in line " + std::to_string(lineCount+1) + ".\nData format is not compatible.\nlen(expected)=" + QString::number(minSize+2).toStdString() + "\nlen(retrieved)=" + QString::number(query.size()).toStdString() + ")");
    else if (data->getBaseLevelMap().isEmpty())
        throw std::runtime_error("Error in line " + std::to_string(lineCount) + ".\nNo baseLevel in data");

    // get timestamp
    bool isInt;
    timestamp = query[timestampIndex].toUInt(&isInt);

    if(!isInt)
        timestamp = data->getTimestampUIntfromString(query[timestampIndex]);

    //              //
    // get vector   //
    //              //
    bool readOk = true;
    // resistances
    for (int i : resistanceIndexMap.keys())
    {
        int rIndex = resistanceIndexMap[i];
        vector[i-1] = query[rIndex].toDouble(&readOk);
    }
    // annotations
    if (userAnnotationIndex != -1)  // user annotation detected
    {
        if (Annotation::isAnnotationString(query[userAnnotationIndex]))
            vector.userAnnotation = Annotation::fromString(query[userAnnotationIndex]);
        else
            throw std::runtime_error("Error in line " + std::to_string(lineCount) + ".\nInvalid annotation string:\n" + query[userAnnotationIndex].toStdString());
    }
    if (detectedAnnotationIndex != -1)
    {
        if (Annotation::isAnnotationString(query[detectedAnnotationIndex]))
            vector.detectedAnnotation = Annotation::fromString(query[detectedAnnotationIndex]);
        else
            throw std::runtime_error("Error in line " + std::to_string(lineCount) + ".\nInvalid annotation string:\n" + query[detectedAnnotationIndex].toStdString());
    }
    // sensor attributes
    for (QString attribute : sensorAttributeMap.keys())
    {
        int index = sensorAttributeMap[attribute];
        vector.sensorAttributes[attribute] = query[index].toDouble(&readOk);
    }

    // formatVersion 0.1: values are relative
    if (formatVersion == "0.1")
        vector = vector.getAbsoluteVector(data->getBaseLevel(timestamp));

    if (!readOk)
        throw std::runtime_error("Error in line " + std::to_string(lineCount) + ".\n");

    // ignore zero vectors
    if (vector != MVector::zeroes())
        data->addMeasurement(timestamp, vector, data->getBaseLevel(timestamp));
}

LeifFileReader::LeifFileReader(QString filePath):
    FileReader(filePath)
{

}

void LeifFileReader::readFile()
{
    // set start_time
    start_time = file.fileTime(QFileDevice::FileTime::FileBirthTime).toTime_t();

    // read header line
    QString line;
    if (!in.readLineInto(&line))
        throw  std::runtime_error(file.fileName().toStdString() + " is empty!");
    lineCount++;

    parseHeader(line);

    // read functionalisation
    if (!in.readLineInto(&line))
        throw  std::runtime_error(file.fileName().toStdString() + " is empty!");
    lineCount++;
    if (line.startsWith(" "));
    parseFuncs(line);

    // read data
    while(in.readLineInto(&line))
    {
        // ignore empty lines
        if (line == "")
            continue;

        lineCount++;
        parseValues(line);
    }
}

FileReader::FileReaderType LeifFileReader::getType()
{
    return FileReaderType::Leif;
}

void LeifFileReader::parseHeader(QString line)
{
    QStringList attributeNames = line.split(" ");

    // save names & indexes of attributes
    QRegularExpression r_reg("^R(\\d*)$");
    QRegularExpression t_reg("^t(\\d*)$");

    for (int i=0; i<attributeNames.size(); i++)
    {
        QString attribute = attributeNames[i];

        // ignore empty strings
        if (attribute == "")
            continue;

        auto r_match = r_reg.match(attribute);
        auto t_match = t_reg.match(attribute);

        // resistance value?
        if (r_match.hasMatch())
            resistanceIndexes[r_match.captured(1).toInt()-1] = i;
        // time?
        else if (t_match.hasMatch())
        {
            if (t_match.captured(1) == "1")
                t_index = i;
        }
        // attribute of additional sensor
        else
            sensorAttributeIndexMap[attribute] = i;
    }

    // check header info
    bool headerOk = true;
    QString errorString = "Incompatible header format:\n";
    if (resistanceIndexes.size() == 0)
    {
        headerOk = false;
        errorString += "No resistance attributes.\n";
    }
    if (t_index == -1)
    {
        headerOk = false;
        errorString += "No measurement time attribute.\n";
    }

    if (!headerOk)
        throw std::runtime_error(errorString.toStdString());

    // prepare MVector class:
    // MVector default size is number of resistance values
    emit resetNChannels(resistanceIndexes.size());
    data->resetNChannels();
    data->addAttributes(sensorAttributeIndexMap.keys().toSet());
}

void LeifFileReader::parseFuncs(QString line)
{
    auto resistanceKeys = resistanceIndexes.keys();
    int maxResChannel = *std::max_element(resistanceKeys.begin(), resistanceKeys.end());

    functionalistation = std::vector<int>(maxResChannel+1, 0);
    QStringList funcValues = line.split(" ");

    bool readOk = true;
    for (int channel : resistanceIndexes.keys())
    {
        functionalistation[channel] = funcValues[resistanceIndexes[channel]].toInt(&readOk);

        if (!readOk)
            throw std::runtime_error("Error in line " + QString::number(lineCount+1).toStdString()+ ".\nExpected integer, got\"" + funcValues[channel].toStdString() + "\"!");
    }

    data->setFunctionalisation(functionalistation);
}

void LeifFileReader::parseValues(QString line)
{
    QStringList values = line.split(" ");

    int resMaxIndex = *std::max_element(resistanceIndexes.begin(), resistanceIndexes.end());
    QList<int> sensorAttributeIndexes = sensorAttributeIndexMap.values();
    int sensAttrMaxIndex = *std::max_element(sensorAttributeIndexMap.begin(), sensorAttributeIndexMap.end());

    // get max index needed
    int max = qMax(t_index, resMaxIndex);
    max = qMax(max, sensAttrMaxIndex);

    // check size of line
    if (values.size() <=max)
        throw std::runtime_error("Error in line " + QString::number(lineCount+1).toStdString()+ ".\nLine has to contain at least " + QString::number(max).toStdString() + " values!");

    double time;
    MVector vector;

    // get time of measurement
    bool conversionOk = false;
    time = values[t_index].toDouble(&conversionOk);

    if (!conversionOk || time<0)
        throw std::runtime_error("Error in line " + std::to_string(lineCount+1) + ".\nIncompatible time value.\n\"" + values[t_index].toStdString() + "\" can not be converted into a double!");

    // read resistance values
    for (int channel : resistanceIndexes.keys())
    {
        int index = resistanceIndexes[channel];
        bool conversionOk;
        vector[channel] = values[index].toDouble(&conversionOk);

        if (!conversionOk)
            throw std::runtime_error("Error in line " + std::to_string(lineCount+1) + ".\nIncompatible resistance value.\n\"" + values[index].toStdString() + "\" can not be converted into a double!");
    }

    // read attributes
    for (QString attribute : sensorAttributeIndexMap.keys())
    {
        int index = sensorAttributeIndexMap[attribute];
        bool conversionOk;
        vector.sensorAttributes[attribute] = values[index].toDouble(&conversionOk);

        if (!conversionOk)
            throw std::runtime_error("Error in line " + std::to_string(lineCount+1) + ".\nIncompatible attribute value.\n\"" + values[index].toStdString() + "\" can not be converted into a double!");
    }

    int timestamp = start_time+qRound(time);
    // base vector is first vector
    if (data->getAbsoluteData().isEmpty())
        data->setBaseLevel(timestamp, vector);

    data->addMeasurement(timestamp, vector);
}
