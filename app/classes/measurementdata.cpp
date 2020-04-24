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

QString MeasurementData::funcName = "Custom";
std::vector<int> MeasurementData::functionalisation(MVector::nChannels, 0);


MeasurementData::MeasurementData(QObject *parent) :
    QObject(parent),
    sensorFailures(MVector::nChannels, false)
{
    // zero init info
    setComment("");
    setSensorId("");
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

/*!
 * \brief MeasurementData::clear deletes all data from MeasurementData
 */
void MeasurementData::clear()
{
    baseLevelMap.clear();
    clearSelection();
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
        else
            saveFilename = "data";
        saveFilename = pathList.join("/") + "/";
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
 * if the vector map was preciously empty, set  \a vector as start timestamp of the measurement
 * \param timestamp
 * \param vector
 */
void MeasurementData::addMeasurement(uint timestamp, MVector vector)
{
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

std::vector<int> MeasurementData::getFunctionalities() const
{
    return functionalisation;
}

void MeasurementData::setFunctionalities(const std::vector<int> &value)
{
    Q_ASSERT(value.size() == MVector::nChannels);

    if (value != functionalisation)
    {
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
    Q_ASSERT(funcs.size() == MVector::nChannels);
    Q_ASSERT(fails.size() == MVector::nChannels);

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
bool MeasurementData::changed() const
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

/*!
 * \brief MeasurementData::saveData saves all vectors added & meta info.
 * The save path & filename is determined by a QFileDialog::getSaveFileName
 */
bool MeasurementData::saveData(QWidget *widget)
{
    return saveData(widget, data);
}

/*!
 * \brief MeasurementData::saveData saves all vectors added & meta info in \a filename
 */
bool MeasurementData::saveData(QWidget *widget, QString filename)
{
    return  saveData(widget, filename, data);
}

/*!
 * \brief MeasurementData::saveData saves all vectors in \a map & meta info.
 */
bool MeasurementData::saveData(QWidget* widget, QMap<uint, MVector> map)
{
    QString path = (saveFilename != "./data/") ? saveFilename : "./data/";
    QString fileName = QFileDialog::getSaveFileName(widget, QString("Save data"), path, "Data files (*.csv)");

    // no file selected
    if (fileName.isEmpty() || fileName.endsWith("/"))
        return false;

    if (fileName.split(".").last() != "csv")
        fileName += ".csv";

    // update saveFilename
    saveFilename = fileName;
    return saveData(widget, fileName, map);
}

/*!
 * \brief MeasurementData::saveData saves all vectors in \a map & meta info in \a filename.
 */
bool MeasurementData::saveData(QWidget* widget, QString filename, QMap<uint, MVector> map)
{
    // filename is directory & based on saveFilename
    if (filename.endsWith("/") && filename == saveFilename)
        return saveData(widget, map);

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(widget, tr("Unable to open file"),
            file.errorString());
        return false;
    }

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
        // classes
        valueList << iter.value().userAnnotation.toString() << iter.value().detectedAnnotation.toString();
        out <<  valueList.join(";") << "\n";

        iter++;
    }
    setDataChanged(false);
    return true;
}

/*!
 * \brief MeasurementData::saveSelection saves current selection. Uses QFileDialog::saveFileDialog to determine save filename.

 */
bool MeasurementData::saveSelection(QWidget *widget)
{
    Q_ASSERT("Selection data is empty!" && !selectedData.isEmpty());
    return saveData(widget, selectedData);
}

/*!
 * \brief MeasurementData::saveSelection  saves current selection in \a filename.
 */
bool MeasurementData::saveSelection(QWidget *widget, QString filename)
{
    Q_ASSERT("Selection data is empty!" && !selectedData.isEmpty());
    return saveData(widget, filename, selectedData);
}

/*!
 * \brief MeasurementData::saveAverageSelectionMeasVector saves average vector of the current selection.
 * \param widget
 * \param filename
 * \return
 */
bool MeasurementData::saveAverageSelectionMeasVector(QWidget *widget, QString filename)
{
    Q_ASSERT("Selection data is empty!" && !selectedData.isEmpty());

    // calculate average selection vector
    MVector selectionMeasVector = getSelectionVector();

    // save average vector in file
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(widget, tr("Unable to open file"),
            file.errorString());
        return false;
    }

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
bool MeasurementData::saveAverageSelectionFuncVector(QWidget *widget, QString filename)
{
    Q_ASSERT("Selection data is empty!" && !selectedData.isEmpty());

    // calculate average selection vector
    MVector selectionVector = getSelectionVector();
    MVector selectionFuncVector = selectionVector.getFuncVector(functionalisation, sensorFailures);

    // save average vector in file
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(widget, tr("Unable to open file"),
            file.errorString());
        return false;
    }

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

/*!
 * \brief MeasurementData::loadData loads file determined by QFileDialog::getOpenFileName.
 */
bool MeasurementData::loadData(QWidget* widget)
{
    bool dataSaved = true;

    // ask to save old data
    if (!data.isEmpty() && dataChanged)
    {
        if (QMessageBox::question(widget, tr("Save data"),
            "Do you want to save the current measurement before loading data?\t") == QMessageBox::Ok)
            dataSaved=saveData(widget);
    }
    if (!dataSaved)
        return false;

    // make data directory
    if (!QDir("data").exists())
        QDir().mkdir("data");

    QString path = (saveFilename != "") ? saveFilename : "./data";
    QString fileName = QFileDialog::getOpenFileName(widget, "Open data file", path, "Data files (*.csv)");

    if (fileName.isEmpty())
    {
        return false;
    } else
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::information(widget, "Unable to open file",
                file.errorString());
            return false;
        }

        // clear data
        clear();
        emit lgClearSelection();
        setComment("");
        setSensorId("");
        setFailures("0000000000000000000000000000000000000000000000000000000000000000");
        emit dataReset();

        // read data from file
        QTextStream in(&file);
        QString line, comment;
        QRegExp rx("\\;");

        // first line: check for version
        QString firstLine = in.readLine();
//        qDebug() << firstLine;

        if (!firstLine.startsWith("#measurement data"))
        {
            QMessageBox::warning(widget, "Can not read file", "The selected file is not a measurement file!");
            return false;
        }
        if (firstLine.startsWith("#measurement data v"))
        {
            savefileFormatVersion = firstLine.right(firstLine.length()-QString("#measurement data v").length());
            savefileFormatVersion = savefileFormatVersion.split(";").join("");
        } else
            savefileFormatVersion = "0.1";


        bool readOk = true;
        bool replotStatusSet = false;
        while (readOk && in.readLineInto(&line))
        {
//            qDebug() << line;
            if (line == "")
                continue;   // ignore
            if (line[0] == '~') // file help
                continue; // ignore
            else if (line[0] == '#') // meta info
                readOk = getMetaData(line);
            else  // data
            {
                if (!replotStatusSet)
                {
                    replotStatusSet = true;
                    emit setReplotStatus(false);
                }
                readOk = getData(line);
            }
        }

        emit setReplotStatus (true);

        // catch errors reading files
        if (!readOk)
        {
            QMessageBox::critical(widget, "Error reading file", "Error in line:\n" + line);
            clear();
            clearSelection();
            return false;
        }

        setDataChanged(false);

        // set filename
        saveFilename = fileName;

        emit labelsUpdated(data);   // reset labels

        return true;
    }
}

/*!
 * \brief MeasurementData::getMetaData is a helper function for extracting meta data from save files.
 */
bool MeasurementData::getMetaData(QString line)
{
    bool  readOk = true;

    if (line.startsWith("#sensorId:"))
    {
        QString sensorId = line.right(line.length()-QString("#sensorId:").length());
        sensorId = sensorId.split(";").join("");
        setSensorId(sensorId);
    }
    else if (line.startsWith("#failures:"))
    {
        QString failureString = line.right(line.length()-QString("#failures:").length());
        failureString=failureString.split(";").join("");
        if (failureString.size() != MVector::nChannels)
        {
            qWarning() << "Failure string not valid. Using empty failure string.";
            failureString = "0000000000000000000000000000000000000000000000000000000000000000";
        }
        setFailures(failureString);
    }
    else if (line.startsWith("#funcName:"))
    {
        funcName = line.split(":")[1];
        emit funcNameSet(funcName);
    }
    else if (line.startsWith("#functionalisation:"))
    {
        QString rawString = line.right(line.length()-QString("#functionalisation:").length());
        QStringList funcList = rawString.split(";");

        std::vector<int> newFunc;
        for (int i=0; i<MVector::nChannels; i++)
            newFunc.push_back(funcList[i].toInt());
        setFunctionalities(newFunc);

        // funcName not stored in file?!
        // -> emit Custom
        if (funcName == "Custom")
        {
            for (int i=0; i<functionalisation.size(); i++)
            {
                if (functionalisation[i] != 0)
                {
                    emit funcNameSet("Custom");
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
            timestamp = getTimestampUIntfromString(valueList[0]);

        // get base level vector
        MVector baseLevel;
        for (int i=0; i<MVector::nChannels; i++)
            baseLevel[i] = valueList[i+1].toDouble();

        setBaseLevel(timestamp, baseLevel);
    }
    else if (line.startsWith("#classes:"))
    {
        QStringList classStringList =  line.right(line.length()-QString("#classes:").length()).split(";");
        for (QString classString : classStringList)
        {
            if (classString == "")
                continue;   // ignore empty classStrings

            if (!aClass::isClassString(classString))
            {
                QMessageBox::critical(static_cast<QWidget*>(this->parent()), "Invalid class name: " + classString, "The class name is invalid!");
                readOk = false;
                break;
            }
            aClass c = aClass::fromString(classString);

            if (!classList.contains(c))
                addClass(c);
        }
    }
    else if (line.startsWith("#header:"))
        ;   // ignore
    else    // comment
        setComment(dataComment + line.right(line.length()-1) + "\n");

    return readOk;
}

/*!
 * \brief MeasurementData::getData is a helper function for extracting vectors from save files.
 */
bool MeasurementData::getData(QString line)
{
    uint timestamp;
    MVector vector;
    bool readOk = true;

    if (line == "") // ignore empty lines
        return true;

    // support old format:
    // - measurement values stored as relative vectors
    if (savefileFormatVersion == "0.1")
    {
        QStringList query = line.split(";");

        // line has to contain vector + [user defined and detected class]
        if ((query.size() < MVector::nChannels+1) || (query.size() > MVector::nChannels+3))
        {
            QMessageBox::critical(static_cast<QWidget*>(this->parent()), "Error loading measurement data", "Data format is not compatible (len=" + QString::number(query.size()) + "):\n" + line);
            return false;
        } else if (baseLevelMap.isEmpty())
        {
            QMessageBox::critical(static_cast<QWidget*>(this->parent()), "Error loading measurement data", "No baseLevel in data");
            return false;
        }
        // prepare vector
        timestamp = query[0].toUInt(&readOk);
        vector;
        for (int i=0; i<MVector::nChannels; i++)
        {
            vector[i] = query[i+1].toDouble(&readOk);
        }
        if (query.size() > MVector::nChannels+1)
        {
            if (Annotation::isAnnotationString(query[MVector::nChannels+1]))
                vector.userAnnotation = Annotation::fromString(query[MVector::nChannels+1]);
            else
            {
                return false;
            }
        }
        if (query.size() > MVector::nChannels+2)
        {
            if (Annotation::isAnnotationString(query[MVector::nChannels+2]))
                vector.detectedAnnotation = Annotation::fromString(query[MVector::nChannels+2]);
            else
            {
                return false;
            }
        }
        vector = vector.getAbsoluteVector(getBaseLevel(timestamp));
    }
    else //if (version == "1.0")   // current version -> default
    {
        QStringList query = line.split(";");

        // line has to contain vector + [user defined and detected class]
        if ((query.size() < MVector::nChannels+1) || (query.size() > MVector::nChannels+3))
        {
            QMessageBox::critical(static_cast<QWidget*>(this->parent()), "Error loading measurement data", "Data format is not compatible (len=" + QString::number(query.size()) + "):\n" + line);
            return false;
        } else if (baseLevelMap.isEmpty())
        {
            QMessageBox::critical(static_cast<QWidget*>(this->parent()), "Error loading measurement data", "No baseLevel in data");
            return false;
        }
        // prepare vector
        // get timestamp: uint or string
        bool isInt;
        timestamp = query[0].toUInt(&isInt);

        if(!isInt)
            timestamp = getTimestampUIntfromString(query[0]);

        for (int i=0; i<MVector::nChannels; i++)
        {
            vector[i] = query[i+1].toDouble(&readOk);
        }
        if (query.size() > MVector::nChannels+1)
        {
            QString userString = query[MVector::nChannels+1];
            if (Annotation::isAnnotationString(userString))
                vector.userAnnotation = Annotation::fromString(userString);
            else
            {
                qWarning() << "Warning: User defined class invalid: \"" << userString << "\"";
            }
        }
        if (query.size() > MVector::nChannels+2)
        {
            QString classString = query[MVector::nChannels+2];
            if (Annotation::isAnnotationString(classString))
                vector.detectedAnnotation = Annotation::fromString(classString);
            else
            {
                qWarning() << "Warning: Detected class invalid: \"" << classString << "\"";
            }
        }

    }

    // ignore zero vectors
    if (vector != MVector::zeroes())
        addMeasurement(timestamp, vector, getBaseLevel(timestamp));

    return readOk;
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

//const MVector MeasurementData::getSelectionVector(QMap<uint, MVector>::iterator begin, QMap<uint, MVector>::iterator end, uint endTimestamp, MultiMode mode)
//{
//    // init vector
//    MVector vector;
//    for (int i=0; i<MVector::nChannels; i++)
//        vector[i] = 0.0;

//    if (mode == MultiMode::Average)
//    {
//    auto iter = begin;
//    int n = 0;

//    // iterate until end; if endTimestamp was set (!= 0) also check for endTimestamp
//    while (iter != end && (endTimestamp==0 || iter.key() <= endTimestamp))
//    {
//        for (int i=0; i<MVector::nChannels; i++)
//            vector[i] += iter.value()[i];

//        iter++;
//        n++;
//    }


//    for (int i=0; i<MVector::nChannels; i++)
//        vector[i] = vector[i] / n;
//    }
//    else
//        Q_ASSERT ("Error: Invalid MultiMode." && false);

//    return vector;
//}

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
    funcName = name;
}
