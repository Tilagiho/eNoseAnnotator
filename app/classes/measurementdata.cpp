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

MeasurementData::MeasurementData(QObject *parent, size_t nChannels) :
    QObject(parent),
    functionalisation(nChannels, 0),
    sensorFailures(nChannels, 0)
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
QMap<uint, RelativeMVector> MeasurementData::getRelativeData()
{
    QMap<uint, RelativeMVector> relativeData;

    for (uint timestamp : data.keys())
        relativeData[timestamp] = data[timestamp].getRelativeVector();

    return relativeData;
}

QMap<uint, RelativeMVector>MeasurementData::getFuncData()
{
    // only one func set
    // -> return full relative data
    if (functionalisation.getFuncMap(sensorFailures).size() == 1)
        return getRelativeData();

    QMap<uint, RelativeMVector> relativeData = getRelativeData();
    QMap<uint, RelativeMVector> funcData;
    for (int timestamp : relativeData.keys())
        funcData[timestamp] = relativeData[timestamp].getFuncVector(functionalisation, sensorFailures, inputFunctionType);

    return funcData;
}

/*!
 * \brief MeasurementData::getAbsoluteData returns a map of the vectors contained in the MeasurementData as absolute vectors
 * \return
 */
const QMap<uint, AbsoluteMVector>& MeasurementData::getAbsoluteData()
{
    return data;
}

/*!
 * \brief MeasurementData::getSelectionMap returns map of the vectors in the current selection
 * \return
 */
const QMap<uint, AbsoluteMVector>& MeasurementData::getSelectionMap()
{
    return selectedData;
}

const QMap<uint, AbsoluteMVector>& MeasurementData::getFitMap()
{
    if (!selectedData.isEmpty())
        return selectedData;

    return data;
}


/*!
 * \brief MeasurementData::clear deletes all data from MeasurementData
 */
void MeasurementData::clear()
{
    clearSelection();

    baseVectorMap.clear();
    data.clear();
    emit dataCleared();

    std::vector<bool> zeroFailures;
    for (int i=0; i<sensorFailures.size(); i++)
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
 * \brief MeasurementData::addVector adds \a vector at \a timestamp
 * if the vector map was previously empty, set  \a vector as start timestamp of the measurement
 * \param timestamp
 * \param vector
 */
void MeasurementData::addVector(uint timestamp, AbsoluteMVector vector)
{
    // sync sensor attributes
    for (QString attributeName : vector.sensorAttributes.keys())
        if (!sensorAttributes.contains(attributeName))
            addAttributes(QStringList{attributeName});

    for (QString attributeName : sensorAttributes)
        if (!vector.sensorAttributes.keys().contains(attributeName))
            vector.sensorAttributes[attributeName] = 0.0;

    checkLimits(vector);

    //  double usage of timestamps:
    if (data.contains(timestamp))
    {
        qCritical() << timestamp << ": double usage of timestamp!";
        throw std::runtime_error("Error adding vector: double usage of timestamp!");
    }

    // set base vector
    vector.setBaseVector(getBaseVector(timestamp));

    // add data, update dataChanged
    data[timestamp] = vector;
    if (!dataChanged)
        setDataChanged(true);

    if (replotStatus)
        emit vectorAdded(timestamp, vector, functionalisation, sensorFailures, true);
}

/*!
 * \brief MeasurementData::addVector adds \a vector at \a timestamp
 * \param timestamp
 * \param vector
 * \param baseLevel
 */
void MeasurementData::addVector(uint timestamp, AbsoluteMVector vector, AbsoluteMVector baseVector)
{
    // if new baseLevel: add to baseLevelMap
    if (baseVectorMap.isEmpty() || baseVector != *getBaseVector(timestamp))
        setBaseVector(timestamp, baseVector);

    // add vector to data
    addVector(timestamp, vector);
}

void MeasurementData::setData(QMap<uint, AbsoluteMVector> absoluteData, QMap<uint, AbsoluteMVector> baseVectors)
{
    // clear data
    data.clear();
    baseVectorMap.clear();

    // set baseVectors
    baseVectorMap = baseVectors;

    // add vectors
    replotStatus = false;
    for (uint timestamp : absoluteData.keys())
        addVector(timestamp, absoluteData[timestamp]);

    emit dataSet(data, functionalisation, sensorFailures);
    replotStatus = true;
}

void MeasurementData::setClasslist(QList<aClass> newClassList)
{
    classList.clear();
    aClass::staticClassSet.clear();

    for (aClass aclass : newClassList)
        addClass(aclass);
}

void MeasurementData::setSensorAttributes(QStringList newSensorAttributes)
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

uint MeasurementData::getStartTimestamp()
{
    return data.keys().first();
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

void MeasurementData::setSensorFailure(uint index, bool value)
{
    auto failures = sensorFailures;
    failures[index] = value;
    setSensorFailures(failures);
}

void MeasurementData::setSensorFailures(const std::vector<bool> &failures)
{
    Q_ASSERT(failures.size() == nChannels());

    if (failures != sensorFailures)
    {
        sensorFailures = failures;
        setDataChanged(true);
        emit sensorFailuresSet(data, functionalisation, sensorFailures);

        AbsoluteMVector stdDevVector;
        if (!selectedData.isEmpty())
            stdDevVector.setBaseVector(selectedData.first().getBaseVector());
        auto selectionVector = getAbsoluteSelectionVector(&stdDevVector);
        emit selectionVectorChanged(selectionVector, stdDevVector, sensorFailures, functionalisation);
    }
}

/*!
 * \brief MeasurementData::setFailures sets sensor failures from a QString.
 * \param failureString is expected to be a numeric string of length MVector::nChannels.
 */
void MeasurementData::setSensorFailures(const QString failureString)
{
    Q_ASSERT(failureString.length()==MVector::nChannels);

    std::vector<bool> failures;

    for (int i=0; i<MVector::nChannels; i++)
    {
        Q_ASSERT(failureString[i]=="0" || failureString[i]=="1");
        failures.push_back(failureString[i] == "1");
    }

    setSensorFailures(failures);
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
void MeasurementData::setBaseVector(uint timestamp, AbsoluteMVector baseVector)
{
    Q_ASSERT (!baseVectorMap.contains(timestamp));

    if (baseVectorMap.isEmpty() || baseVectorMap.last() != baseVector)
    {
        baseVectorMap.insert(timestamp, baseVector);
        setDataChanged(true);
//        qDebug() << "New baselevel at " << timestamp << ":\n" << baseLevelMap[timestamp].toString();
    }
}

QList<aClass> MeasurementData::getClassList() const
{
    return classList;
}

Functionalisation MeasurementData::getFunctionalisation() const
{
    return functionalisation;
}

void MeasurementData::setFunctionalisation(const Functionalisation &value)
{
    Q_ASSERT(value.size() == MVector::nChannels);

    if (value != functionalisation)
    {
        functionalisation = value;

        // emit changes
        setDataChanged(true);
        emit functionalisationChanged();

        AbsoluteMVector stdDevVector;
        if (!selectedData.isEmpty())
            stdDevVector.setBaseVector(selectedData.first().getBaseVector());
        auto selectionVector = getAbsoluteSelectionVector(&stdDevVector);
        emit selectionVectorChanged(selectionVector, stdDevVector, sensorFailures, functionalisation);    }

        // set func name
        if (functionalisation.getName() == "None")
        {
            for (uint i=0; i<MVector::nChannels; i++)
                if (functionalisation[i] != 0)
                    setFuncName("Custom");
        }

        // check for NC funcs
        for (uint i=0; i<MVector::nChannels; i++)
        {
            if (functionalisation[i] == FUNC_NC_VALUE && !sensorFailures[i])
                setSensorFailure(i, true);
        }
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
AbsoluteMVector* MeasurementData::getBaseVector(uint timestamp)
{
    if (baseVectorMap.isEmpty())
        throw std::runtime_error("Error: No baselevel was set!");

    uint bsTimestamp = 0;

    //  go through baseLevelMap and store ts in bsTimestamp until timestamp < ts
    // -> bsTimestamp is last ts
    // or until timestamp == ts
    // -> bsTimestamp = ts
    for (auto  ts: baseVectorMap.keys())
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
    return &baseVectorMap[bsTimestamp];
}

std::vector<bool> MeasurementData::getSensorFailures() const
{
    return sensorFailures;
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
    // workaround because QMap<uint, AbsoluteMVector> cannot be converted to QMap<uint, MVector> for some reason
    QMap<uint, MVector>* neutralData = (QMap<uint, MVector>*) &data;
    return saveData(filename, *neutralData);
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
    out << "#funcName:" << functionalisation.getName() << "\n";
    QStringList funcList;
    for (int i=0; i<functionalisation.size(); i++)
        funcList << QString::number(functionalisation[i]);
    out << "#functionalisation:" << funcList.join(";") << "\n";

    // base vector
    for (uint timestamp : baseVectorMap.keys())
    {
        out << "#baseLevel:" << getTimestampStringFromUInt(timestamp) << ";";
        QStringList valueList;
        for (int i=0; i<MVector::nChannels; i++)
            valueList << QString::number(baseVectorMap[timestamp][i], 'g', 10);
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
 * \brief saveSelectionVector saves a
 * \param saveFunc
 */
void MeasurementData::saveSelectionVector(QString filePath, bool saveFunc)
{
    RelativeMVector stdDevVector;
    auto selectionVector = getRelativeSelectionVector(&stdDevVector);

    if (saveFunc)
    {
        stdDevVector = stdDevVector.getFuncVector(functionalisation, sensorFailures);
        selectionVector = selectionVector.getFuncVector(functionalisation, sensorFailures);
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        throw std::runtime_error("Unable to open file: " + file.errorString().toStdString());

    QTextStream out(&file);
    if (saveFunc)
        out << "func;value;standard deviation\n";
    else
        out << "channel;value;standard deviation\n";

    for (size_t i=0; i<selectionVector.getSize(); i++)
    {
        QStringList output;

        if (saveFunc)
            output << QString::number(functionalisation.getFuncMap().keys()[i]);
        else
            output << QString::number(i+1);
        output << QString::number(selectionVector[i]) << QString::number(stdDevVector[i]);

        out << output.join(";") << "\n";
    }
}


/*!
 * \brief MeasurementData::saveSelection  saves current selection in \a filename.
 */
bool MeasurementData::saveSelection(QString filename)
{
    Q_ASSERT("Selection data is empty!" && !selectedData.isEmpty());

    // workaround because QMap<uint, AbsoluteMVector> cannot be converted to QMap<uint, MVector> for some reason
    QMap<uint, MVector>* neutralSelectionData = (QMap<uint, MVector>*) &selectedData;
    return saveData(filename, *neutralSelectionData);
}

/*!
 * \brief MeasurementData::saveAverageSelectionMeasVector saves average vector of the current selection.
 * \param widget
 * \param filename
 * \return
 */
bool MeasurementData::saveAverageSelectionVector(QString filename, bool saveAbsolute)
{
    Q_ASSERT("Selection data is empty!" && !selectedData.isEmpty());

    // calculate average selection vector
    AbsoluteMVector selectionMeasVector = getAbsoluteSelectionVector();

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
bool MeasurementData::saveAverageSelectionFuncVector(QString filename, bool saveAbsolute)
{
    Q_ASSERT("Selection data is empty!" && !selectedData.isEmpty());

    // calculate average selection vector
    AbsoluteMVector selectionVector = getAbsoluteSelectionVector();
    RelativeMVector selectionFuncVector = selectionVector.getFuncVector(functionalisation, sensorFailures, inputFunctionType);

    // save average vector in file
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly))
        throw std::runtime_error("Unable to open file:" + file.errorString().toStdString());

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

void MeasurementData::saveLabViewFile(QString filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly))
        throw std::runtime_error("Unable to open file:" + file.errorString().toStdString());

    QTextStream out(&file);

    // write header
    QStringList header;

    auto sensorAttributeList = sensorAttributes;
    header << sensorAttributeList;
    for (size_t i=0; i<nChannels(); i++) {
        header << "t" + QString::number(i+1);
        header << "R" + QString::number(i+1);
    }
    out << header.join(" ") << "\n";

    // write functionalisation
    QString funcPrefix = "                 ";   // func line begins with 17 whitespaces
    QStringList funcList;
    for (size_t i=0; i<nChannels(); i++) {
        funcList << QString::number(functionalisation[i]);
    }
    out << funcPrefix + funcList.join("  ") << "\n";   // two whitespaces on purpose

    if (!data.isEmpty()) {
        auto startTimestamp = data.firstKey();
        // write measurement start
        if (startTimestamp > 10000) {  // don't write out invalid timestamps (too small)
            QString measStartPrefix = "meas_start:";
            QString measStart = getTimestampStringFromUInt(data.firstKey());
            out << measStartPrefix + measStart << "\n";
        }

        // write data
        for (auto timestamp : data.keys())
        {
            AbsoluteMVector vector = data[timestamp];
            QStringList valueList;

            // additional sensors
            for (auto attribute : sensorAttributeList)
                valueList << QString::number(vector.sensorAttributes[attribute], 'f', 2);

            // t & R pairs
            for (size_t i=0; i<vector.getSize(); i++) {
                valueList << QString::number(timestamp - startTimestamp, 'f', 2);
                valueList << QString::number(vector[i], 'f', 0);
            }
            out << valueList.join(" ") << "\n";
        }
    }


}

void MeasurementData::copyFrom(MeasurementData *otherMData)
{
    // sensorFailures and functionalisation are static
    // -> deleted when clear() is called
    // --> remember sensorFailures and functionalisation
    // TODO: introduce non-static sensorFailures and functionalisation

    // clear local data & reset number of channels
    clear();
    resetNChannels();

    // meta data
    setClasslist(otherMData->getClassList());
    setSensorAttributes(otherMData->getSensorAttributes());
    setSaveFilename(otherMData->getSaveFilename());
    setComment(otherMData->getComment());
    setSensorId(otherMData->sensorId);

    setSensorFailures(otherMData->getSensorFailures());
    setFunctionalisation(otherMData->getFunctionalisation());

    // data
    setData(otherMData->getAbsoluteData(), otherMData->getBaseLevelMap());
    setDataChanged(false);
}

void MeasurementData::setSelection(uint lower, uint upper)
{
    // ignore existing selections
    if (!selectedData.isEmpty() && selectedData.firstKey() == lower && selectedData.lastKey() == upper)
        return;

    // clear selectedData
    clearSelection();

    // selection deselected
    if (upper < lower)
        return;

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
        QMap<uint, AbsoluteMVector>::iterator beginIter, endIter;
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

//    qDebug() << "Selection made: " << selectedData.firstKey() << ", " << selectedData.lastKey() << "\n" << vector.toString() << "\n";
    AbsoluteMVector stdDevVector;
    if (!selectedData.isEmpty())
        stdDevVector.setBaseVector(selectedData.first().getBaseVector());
    auto selectionVector = getAbsoluteSelectionVector(&stdDevVector);
    emit selectionVectorChanged(selectionVector, stdDevVector, sensorFailures, functionalisation);
}

const AbsoluteMVector MeasurementData::getAbsoluteSelectionVector(MVector *stdDevVector, MultiMode mode)
{
    // no selection made:
    // return zero vector
    if (selectedData.isEmpty())
        return AbsoluteMVector();

    // only average supported
    Q_ASSERT(mode == MultiMode::Average);

    AbsoluteMVector selectionVector(getBaseVector(selectedData.firstKey()));

    for (auto timestamp : selectedData.keys())
    {
        // ignore zero vectors
        if (selectedData[timestamp].isZeroVector())
            continue;

        // get absolute selection vector
        AbsoluteMVector vector = selectedData[timestamp];

        // calculate average
        if (mode == MultiMode::Average)
            for (int i=0; i<MVector::nChannels; i++)
                selectionVector[i] += vector[i] / selectedData.size();
    }

    // calculate standard deviation of selection
    if (stdDevVector != nullptr)
    {
        MVector varVector(selectionVector.getBaseVector());

        for (auto timestamp : selectedData.keys())
            varVector += (selectedData[timestamp] - selectionVector).squared();

        *stdDevVector = (varVector / selectedData.size()).squareRoot();
    }

    // set failing channels to zero
    for (size_t i=0; i<selectionVector.getSize(); i++)
    {
        if (sensorFailures[i])
        {
            selectionVector[i] = 0;
            (*stdDevVector)[i] = 0;
        }
    }

    return selectionVector;
}

const RelativeMVector MeasurementData::getRelativeSelectionVector(MVector *stdDevVector, MultiMode mode)
{
    AbsoluteMVector absStdDevVector;
    auto absSelectionVector = getAbsoluteSelectionVector(&absStdDevVector);

    *stdDevVector = absStdDevVector.getRelativeVector() + 100.;
    auto selectionVector = absSelectionVector.getRelativeVector();

    // set failing channels to zero
    for (size_t i=0; i<selectionVector.getSize(); i++)
    {
        if (sensorFailures[i])
        {
            selectionVector[i] = 0;
            (*stdDevVector)[i] = 0;
        }
    }

    return  selectionVector;

//    Q_ASSERT(!selectedData.isEmpty());

//    // only average supported
//    Q_ASSERT(mode == MultiMode::Average);

//    RelativeMVector selectionVector(getBaseVector(selectedData.firstKey()));
//    // zero init
//    for (int i=0; i<MVector::nChannels; i++)
//        selectionVector[i] = 0.0;

//    for (auto timestamp : selectedData.keys())
//    {
//        // ignore zero vectors
//        if (selectedData[timestamp].isZeroVector())
//            continue;

//        // get relative selection vector
//        RelativeMVector vector = selectedData[timestamp].getRelativeVector();

//        // calculate average
//        if (mode == MultiMode::Average)
//            for (int i=0; i<MVector::nChannels; i++)
//                selectionVector[i] += vector[i] / selectedData.size();
//    }

//    // calculate standard deviation of selection
//    if (stdDevVector != nullptr)
//    {
//        MVector varVector;

//        for (auto timestamp : selectedData.keys())
//            varVector += (selectedData[timestamp] - selectionVector).squared();

//        *stdDevVector = (varVector / selectedData.size()).squareRoot();
//    }

//    return selectionVector;
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
    Q_ASSERT(!selectedData.isEmpty());

    data[timestamp].userAnnotation = annotation;
    selectedData[timestamp].userAnnotation = annotation;

    setDataChanged(true);
    QMap<uint, Annotation> changedMap;
    changedMap[timestamp] = annotation;
    emit annotationsChanged(changedMap, true);


}

void MeasurementData::setUserAnnotationOfSelection(Annotation annotation)
{
    Q_ASSERT(!selectedData.isEmpty());

    QMap<uint, Annotation> changedMap;
    for (auto timestamp : selectedData.keys())
    {
        // add to data and selectedData
        data[timestamp].userAnnotation = annotation;
        selectedData[timestamp].userAnnotation = annotation;

        changedMap[timestamp] = annotation;
    }

    setDataChanged(true);
    emit annotationsChanged(changedMap, true);

}

void MeasurementData::setDetectedAnnotation(Annotation annotation, uint timestamp)
{
    data[timestamp].detectedAnnotation = annotation;
    selectedData[timestamp].detectedAnnotation = annotation;

    setDataChanged(true);
    QMap<uint, Annotation> changedMap;
    changedMap[timestamp] = annotation;
    emit annotationsChanged(changedMap, false);
}

void MeasurementData::setDetectedAnnotationOfSelection(Annotation annotation)
{
    QMap<uint, Annotation> changedMap;

    for (auto timestamp : selectedData.keys())
    {
        // data and selectedData
        data[timestamp].detectedAnnotation = annotation;
        selectedData[timestamp].detectedAnnotation = annotation;

        changedMap[timestamp] = annotation;
    }

    setDataChanged(true);
    emit annotationsChanged(changedMap, false);
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
    QMap<uint, Annotation> userAnnotationChangedMap;
    QMap<uint, Annotation> detectedAnnotationChangedMap;

    for (uint timestamp: data.keys())
    {
        if (data[timestamp].userAnnotation.contains(oldClass))
        {
            data[timestamp].userAnnotation.remove(oldClass);
            userAnnotationChangedMap[timestamp] = data[timestamp].userAnnotation;
        }
        if (data[timestamp].detectedAnnotation.contains(oldClass))
        {
            data[timestamp].detectedAnnotation.remove(oldClass);
            detectedAnnotationChangedMap[timestamp] = data[timestamp].detectedAnnotation;
        }
    }

    setDataChanged(true);

    if (!userAnnotationChangedMap.isEmpty())
        emit annotationsChanged(userAnnotationChangedMap, true);
    if (!detectedAnnotationChangedMap.isEmpty())
        emit annotationsChanged(detectedAnnotationChangedMap, false);
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
    QMap<uint, Annotation> userAnnotationChangedMap;
    QMap<uint, Annotation> detectedAnnotationChangedMap;


    for (uint timestamp: data.keys())
    {
        if (data[timestamp].userAnnotation.contains(oldClass))
        {
            data[timestamp].userAnnotation.changeClass(oldClass, newClass);
            userAnnotationChangedMap[timestamp] = data[timestamp].userAnnotation;
        }
        if (data[timestamp].detectedAnnotation.contains(oldClass))
        {
            data[timestamp].detectedAnnotation.changeClass(oldClass, newClass);
            detectedAnnotationChangedMap[timestamp] = data[timestamp].detectedAnnotation;
        }
    }

    setDataChanged(true);

    if (!userAnnotationChangedMap.isEmpty())
        emit annotationsChanged(userAnnotationChangedMap, true);
    if (!detectedAnnotationChangedMap.isEmpty())
        emit annotationsChanged(detectedAnnotationChangedMap, true);
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
    if (!dateTime.isValid())
        throw std::runtime_error(("Invalid timestamp string: " + string).toStdString());

    return dateTime.toTime_t();
}

void MeasurementData::setFuncName(QString name)
{
    if (name != functionalisation.getName())
    {
        functionalisation.setName(name);
        emit functionalisationChanged();
    }
}

void MeasurementData::addAttributes(QStringList newAttributeNames)
{
    for (QString attributeName : newAttributeNames)
        Q_ASSERT(!sensorAttributes.contains(attributeName));

    // add attributes to sensorAttributes
    for (QString newAttribute : newAttributeNames)
        sensorAttributes.append(newAttribute);

    // add to vectors
    for (AbsoluteMVector vector : data)
        for (QString attributeName : newAttributeNames)
            vector.sensorAttributes[attributeName] = 0.0;
}

void MeasurementData::deleteAttributes(QSet<QString> attributeNames)
{
    for (QString attributeName : attributeNames)
        Q_ASSERT(sensorAttributes.contains(attributeName));

    // delete attributes from sensorAttributes
    for (QString attributeName : attributeNames)
        sensorAttributes.removeAll(attributeName);  // each elemtent should only be contained once

    // delete from vectors
    for (AbsoluteMVector vector : data)
        for (QString attributeName : attributeNames)
            vector.sensorAttributes.remove(attributeName);
}

void MeasurementData::renameAttribute(QString oldName, QString newName)
{
    Q_ASSERT(sensorAttributes.contains(oldName));
    Q_ASSERT(!sensorAttributes.contains(newName));
    Q_ASSERT(oldName != newName);

    // rename in sensorAttributes
    sensorAttributes.append(newName);
    sensorAttributes.removeAll(oldName);

    // rename in vectors
    for (AbsoluteMVector vector : data)
    {
        vector.sensorAttributes[newName] = vector.sensorAttributes[oldName];
        vector.sensorAttributes.remove(oldName);
    }
}

void MeasurementData::resetNChannels(size_t channels)
{
    Q_ASSERT(data.isEmpty());

    sensorFailures = std::vector<bool>(channels, false);
    functionalisation = Functionalisation(channels, 0);

    emit sensorFailuresSet(data, functionalisation, sensorFailures);
    emit functionalisationChanged();
}

void MeasurementData::setInputFunctionType(const InputFunctionType &value)
{
    inputFunctionType = value;
}

/*!
 * \brief MeasurementData::getNextTimestamp returns next timestamp contained in data >= timestamp.
 * Returns 0 if no timestamp in data >= timestamp.
 * \param timestamp
 * \return
 */
uint MeasurementData::getNextTimestamp(uint timestamp)
{
    uint nextTimestamp = timestamp;

    while (nextTimestamp <= data.keys().last()) {
        if (data.contains(nextTimestamp))
            return nextTimestamp;

        nextTimestamp++;
    }
    return 0;
}

/*!
 * \brief MeasurementData::getPreviousTimestamp returns previous timestamp contained in data <= timestamp.
 * Returns 0 if no timestamp in data <= timestamp.
 * \param timestamp
 * \return
 */
uint MeasurementData::getPreviousTimestamp(uint timestamp)
{
    uint previousTimestamp = timestamp;

    while (previousTimestamp >= data.keys().first()) {
        if (data.contains(previousTimestamp))
            return previousTimestamp;

        previousTimestamp--;
    }
    return 0;
}

void MeasurementData::checkLimits (const AbsoluteMVector &vector)
{
    Q_ASSERT(vector.getSize() == sensorFailures.size());

    if (!useLimits)
        return;

    auto newSensorFailures = sensorFailures;

    for (size_t i=0; i<vector.getSize(); i++)
        newSensorFailures[i] = vector[i] < lowerLimit || vector[i] > upperLimit;

    if (newSensorFailures != sensorFailures)
        setSensorFailures(newSensorFailures);
}

void MeasurementData::checkLimits ()
{
    // clear sensorFailures
    setSensorFailures(std::vector<bool>(sensorFailures.size(), false));

    // check limits
    for (auto vector : data)
        checkLimits(vector);
}

void MeasurementData::setLimits(double newLowerLimit, double newUpperLimit, bool newUseLimits)
{
    // recalc sensor failure flags if limits or useLimits changed
    bool limitsChanged = newUseLimits && (!qFuzzyCompare(newUpperLimit, upperLimit) || !qFuzzyCompare(newUpperLimit, lowerLimit));
    bool useLimitsChanged = newUseLimits != useLimits;

    auto newSensorFailures = sensorFailures;

    // 4 cases for change
    // 1. useLimits: true -> false:
    //      set all sensorFailureFlags added by limit violations to false
    // 2. useLimits: false -> true:
    //      find all limit violations and set the according flags to true
    // 3. limits: minVal gets bigger or maxVal smaller
    //      find old violations that are no violations anymore and set flag to false
    // 4. limits: minVal gets smaller or maxVal bigger
    //      find new violations that were no violations and set flag  to true
    if (limitsChanged || useLimitsChanged)
    {
        // case 1+2
        if (useLimitsChanged)
        {
            for (MVector vector : data)
            {
                for (size_t i = 0; i<MVector::nChannels; i++)
                {
                    if (vector[i] < newLowerLimit || vector[i] > newUpperLimit)
                        newSensorFailures[i] = newUseLimits;   // useLimits == true -> set flags, else delete them
                }
            }
        } else  // limitsChanged -> case 3+4
        {
            for (MVector vector : data)
            {
                for (size_t i = 0; i<MVector::nChannels; i++)
                {
                    // check lower limit
                    if (vector[i] >= newLowerLimit && vector[i] < lowerLimit)   // case 3
                        newSensorFailures[i] = false;
                    else if (vector[i] < newLowerLimit && vector[i] >= lowerLimit)   // case 4
                            newSensorFailures[i] = true;

                    // check upper limit
                    if (vector[i] <= newUpperLimit && vector[i] > upperLimit)   // case 3
                            newSensorFailures[i] = false;
                    else if (vector[i] > newUpperLimit && vector[i] <= upperLimit) // case 4
                            newSensorFailures[i] = true;
                }
            }
        }

        // set new values
        setSensorFailures(newSensorFailures);
    }

    // save settings
    lowerLimit = newLowerLimit;
    upperLimit = newUpperLimit;
    useLimits = newUseLimits;

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue(USE_LIMITS_KEY, newUseLimits);
    settings.setValue(LOWER_LIMIT_KEY, newLowerLimit);
    settings.setValue(UPPER_LIMIT_KEY, newUpperLimit);
}

bool MeasurementData::getUseLimits() const
{
    return useLimits;
}

size_t MeasurementData::nChannels() const
{
    Q_ASSERT(functionalisation.size() == sensorFailures.size());
    return functionalisation.size();
}

double MeasurementData::getUpperLimit() const
{
    return upperLimit;
}

double MeasurementData::getLowerLimit() const
{
    return lowerLimit;
}

QMap<uint, AbsoluteMVector> MeasurementData::getBaseLevelMap() const
{
    return baseVectorMap;
}

QStringList MeasurementData::getSensorAttributes() const
{
    return sensorAttributes;
}

FileReader::FileReader(QString filePath, QObject* parentWidget):
    file(filePath)
{
    Q_ASSERT(!filePath.isEmpty());

    data = new MeasurementData(parentWidget);
    data->setSaveFilename(filePath);

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
        return new LabviewFileReader(file.fileName());
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
        functionalistation.setName(funcName);
    }
    else if (line.startsWith("#functionalisation:"))
    {
        QString rawString = line.right(line.length()-QString("#functionalisation:").length());
        QStringList funcList = rawString.split(";");

        std::vector<int> funcVector;
        for (int i=0; i<funcList.size(); i++)
            funcVector.push_back(funcList[i].toInt());

        functionalistation.setVector(funcVector);
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
        AbsoluteMVector baseLevel(nullptr, valueList.size()-1);
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
                data->addAttributes(QStringList{headerItem});
            }
        }
        // after parsing the header:
        // reset number of channels
        data->resetNChannels(resistanceIndexMap.size());
        emit resetNChannels(resistanceIndexMap.size()); // resets MVector::nChannels if connected

        // set data meta attributes
        data->setSensorFailures(failureString);
        data->setFunctionalisation(functionalistation);

        for (uint timestamp : baseLevelMap.keys())
            data->setBaseVector(timestamp, baseLevelMap[timestamp]);

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
        vector = static_cast<RelativeMVector>(vector).getAbsoluteVector();

    if (!readOk)
        throw std::runtime_error("Error in line " + std::to_string(lineCount) + ".\n");

    // ignore zero vectors
    if (!vector.isZeroVector())
        data->addVector(timestamp, vector);
}

LabviewFileReader::LabviewFileReader(QString filePath):
    FileReader(filePath)
{
}

void LabviewFileReader::readFile()
{
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
    parseFuncs(line);
    // store pos in case no measurement start line present
    auto measStartPos = in.pos();

    // optional: read measurement start
    if (!in.readLineInto(&line))
        throw  std::runtime_error(file.fileName().toStdString() + " is empty!");
    if (line.startsWith("meas_start:")) {
        parseMeasurementStart(line);
        lineCount++;
    } else {   // reset text stream
        in.seek(measStartPos);
    }

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

FileReader::FileReaderType LabviewFileReader::getType()
{
    return FileReaderType::Leif;
}

void LabviewFileReader::parseHeader(QString line)
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
    data->resetNChannels(resistanceIndexes.size());
    data->addAttributes(sensorAttributeIndexMap.keys());
}

/*!
 * \brief LabviewFileReader::parseFuncs
 * if number of funcs is smaller than number of channels, func is repeated until func size is equal to number of channels
 * -> multiple detectors in one measurement are interpreted as one detector with many channels
 * \param line
 */
void LabviewFileReader::parseFuncs(QString line)
{
    auto resistanceKeys = resistanceIndexes.keys();
    size_t maxResChannel = *std::max_element(resistanceKeys.begin(), resistanceKeys.end()) + 1;

    functionalistation = Functionalisation(maxResChannel, 0);
    QStringList funcValues = line.split(" ");
    funcValues.removeAll("");
    if (funcValues.size() > resistanceKeys.size())
        throw std::runtime_error("Error parsing functionalisation: More func values than resistance values!");
    // fill up funcValues until it has the same number of elements as resistanceKeys
    int originalNFuncs = funcValues.size();
    int i=0;
    while (funcValues.size() != resistanceKeys.size()) {
        funcValues.append(funcValues[i%originalNFuncs]);
        i++;
    }

    bool readOk = true;
    for (size_t channel : resistanceIndexes.keys())
    {
        functionalistation[channel] = funcValues[channel].toInt(&readOk);

        if (!readOk)
            throw std::runtime_error("Error in line " + QString::number(lineCount+1).toStdString()+ ".\nExpected integer, got\"" + funcValues[channel].toStdString() + "\"!");
    }

    data->setFunctionalisation(functionalistation);
}

void LabviewFileReader::parseMeasurementStart(QString line)
{
    QString prefix("meas_start:");
    QString measTimestampString = line.mid(prefix.size(), line.size()-prefix.size());
    start_time = MeasurementData::getTimestampUIntfromString (measTimestampString);
}

void LabviewFileReader::parseValues(QString line)
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

    uint time;
    MVector vector;

    // get time of measurement
    bool conversionOk = false;
    time = static_cast<uint>(qRound(values[t_index].toDouble(&conversionOk)));

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

    uint timestamp = start_time + time;
    // base vector is first vector
    if (data->getAbsoluteData().isEmpty())
        data->setBaseVector(timestamp, vector);

    data->addVector(timestamp, vector);
}
