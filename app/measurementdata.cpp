#include "measurementdata.h"
#include <QDateTime>
#include <QVector>
#include <QtMath>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

MeasurementData::MeasurementData(QObject *parent) : QObject(parent)
{
    // zero init info
    setComment("");
    setSensorId("");
    setFailures("0000000000000000000000000000000000000000000000000000000000000000");
    for (int i=0; i<functionalisation.size(); i++)
        functionalisation[i] = 0;
}

const QMap<uint, MVector> MeasurementData::getRelativeData()
{
    return data;
}

const QMap<uint, MVector> MeasurementData::getAbsoluteData()
{
    QMap<uint, MVector> absoluteMap;
    for (uint timestamp : data.keys())
    {
        MVector absoluteVector;

        for (int i=0; i<MVector::size; i++)
            absoluteVector.array[i] = (1+data[timestamp].array[i]) * getBaseLevel(timestamp).array[i];

        absoluteMap[timestamp] = absoluteVector;
    }

    return absoluteMap;
}

const QMap<uint, MVector> MeasurementData::getSelectionMap()
{
    return selectedData;
}

void MeasurementData::clear()
{
    baseLevelMap.clear();
    clearSelection();
    data.clear();
    std::array<bool, 64> zeroFailures;
    for (int i=0; i<MVector::size; i++)
        zeroFailures[i] = false;
    setSensorFailures(zeroFailures);

    setComment("");
    setSensorId("");


    dataChanged = false;
}

void MeasurementData::clearSelection()
{
    selectedData.clear();
    emit selectionCleared();
}

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

    for (int i=0; i<MVector::size; i++)
        deviationVector.array[i] = 100.0 * (vector.array[i] / baseLevelVector.array[i] - 1.0);


    // add data, update dataChanged
    data[timestamp] = deviationVector;
    if (!dataChanged)
        dataChanged = true;

    // save absolute vectors
    if (saveRawInput)
    {
        uint startTs = data.firstKey();

        QFile f("raw_input_" + QDateTime::fromTime_t(startTs).toString("dMMyyyy-h:mm:ss") + ".csv");
        if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
            QTextStream stream(&f);

            // write header
            if (startTs == timestamp)
            {
                QStringList headerList;
                headerList << "timestamp";

                for (int i=0; i<MVector::size; i++)
                    headerList << "channel" + QString::number(i+1);

                stream << headerList.join(";") << "\n";
            }

            stream << QDateTime::fromTime_t(data.firstKey()).toString("d.MM.yyyy-h:mm:ss") << ";" << vector.toString() << "\n";
            f.close();
        }
    }

//    qDebug() << timestamp << "Added measurement!";

    emit dataAdded(deviationVector, timestamp, true);
    emit absoluteDataAdded(vector, timestamp, true);
}

void MeasurementData::addMeasurement(uint timestamp, MVector vector, MVector baseLevel)
{
    MVector absoluteVector;

    for (int i=0; i<MVector::size; i++)
        absoluteVector.array[i] = (1 + vector.array[i]/100.0) * baseLevel.array[i];

    addMeasurement(timestamp, absoluteVector);
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
        dataChanged = true;
        emit commentSet(dataComment);
    }
}

void MeasurementData::setFailures(std::array<bool, 64> failures)
{
    if (failures != sensorFailures)
    {
        sensorFailures = failures;
        dataChanged = true;
        emit sensorFailuresSet(failures);
    }
}

void MeasurementData::setFailures(QString failureString)
{
    Q_ASSERT(failureString.length()==64);

    std::array<bool, 64> failures;

    for (int i=0; i<MVector::size; i++)
    {
        Q_ASSERT(failureString[i]=="0" || failureString[i]=="1");
        failures[i] = failureString[i] == "1";
    }

    setFailures(failures);
}

void MeasurementData::setSensorId(QString newSensorId)
{
    if (newSensorId != sensorId)
    {
        sensorId = newSensorId;
        dataChanged = true;
        emit sensorIdSet(sensorId);
    }
}

void MeasurementData::setBaseLevel(uint timestamp, MVector baseLevel)
{
    Q_ASSERT (!baseLevelMap.contains(timestamp));

    if (baseLevelMap.isEmpty() || baseLevelMap.last() != baseLevel)
    {
        baseLevelMap.insert(timestamp, baseLevel);
        dataChanged = true;
        qDebug() << "New baselevel at " << timestamp << ":\n" << baseLevelMap[timestamp].toString();
    }
}

QList<aClass> MeasurementData::getClassList() const
{
    return classList;
}

bool MeasurementData::getSaveRawInput() const
{
    return saveRawInput;
}

void MeasurementData::setSaveRawInput(bool value)
{
    saveRawInput = value;
}

std::array<int, MVector::size> MeasurementData::getFunctionalities() const
{
    return functionalisation;
}

void MeasurementData::setFunctionalities(const std::array<int, MVector::size> &value)
{
    if (value != functionalisation)
    {
        functionalisation = value;
        dataChanged = true;
    }
}

bool MeasurementData::changed() const
{
    return dataChanged;
}

MVector MeasurementData::getBaseLevel(uint timestamp)
{
    if (baseLevelMap.isEmpty())
    {
        qWarning() << "No baselevel was set!" << "\n";
        return  MVector::zeroes();
    }

    uint bsTimestamp = 0;

    //  go through baseLevelMap and store ts in bsTimestamp until timestamp < ts
    // -> bsTimestamp is
    for (auto  ts: baseLevelMap.keys())
    {
        if (ts > timestamp)
            break;

        bsTimestamp = ts;
    }
    Q_ASSERT ("Error: No baselevel was set!" && bsTimestamp!=0);

    return baseLevelMap[bsTimestamp];
}

std::array<bool, 64> MeasurementData::getSensorFailures() const
{
    return sensorFailures;
}

void MeasurementData::setSensorFailures(const std::array<bool, 64> &value)
{
    if (value != sensorFailures)
    {
        sensorFailures = value;
        dataChanged = true;
        emit sensorFailuresSet(value);
    }
}

QString MeasurementData::getSensorId() const
{
    return sensorId;
}

QString MeasurementData::getFailureString()
{
    QString failureString("");

    for (int i=0; i<MVector::size; i++)
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

bool MeasurementData::saveData(QWidget *widget)
{
    return saveData(widget, data);
}

bool MeasurementData::saveData(QWidget* widget, QMap<uint, MVector> map)
{
    QString fileName = QFileDialog::getSaveFileName(widget, QString("Save data"), QString(""), "Data files (*.csv)");

    if (fileName.isEmpty())
    {
        return false;
    } else
    {

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::information(widget, tr("Unable to open file"),
                file.errorString());
            return false;
        }

        QTextStream out(&file);
        // write info
        // version
        out << "#measurement data " << version << "\n";

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
        QStringList funcList;
        for (int i=0; i<functionalisation.size(); i++)
            funcList << QString::number(functionalisation[i]);
        out << "#functionalisation:" << funcList.join(";") << "\n";

        // baseLevel
        for (uint timestamp : baseLevelMap.keys())
        {
            out << "#baseLevel:" << timestamp << ";";
            QStringList valueList;
            for (int i=0; i<MVector::size; i++)
                valueList << QString::number(baseLevelMap[timestamp].array[i], 'g', 10);
            out <<  valueList.join(";") << "\n";

        }

        // write data
        auto iter = map.begin();

        while (iter != map.end())
        {
            QStringList valueList;
            valueList << QString::number(iter.key(), 'g', 10);      // timestamp

            // vector
            for (int i=0; i<MVector::size; i++)
                valueList << QString::number(iter.value().array[i], 'g', 10);
            out <<  valueList.join(";") << "\n";

            iter++;
        }
        dataChanged = false;
        return true;
    }

}

bool MeasurementData::saveSelection(QWidget *widget)
{
    if (selectedData.isEmpty())
    {
        QMessageBox::information(widget, tr("Save failed"),
            "Please select data first!\t");
        return false;
    }
    return saveData(widget, selectedData);
}

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

    QString fileName = QFileDialog::getOpenFileName(widget, "Open data file", "", "Data files (*.csv)");

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
        clearSelection();
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
        qDebug() << firstLine;

        if (!firstLine.startsWith("#measurement data "))
        {
            QMessageBox::warning(widget, "Can not read file", "The selected file is not a measurement file!");
            return false;
        }
        version = line.right(line.length()-QString("#measurement data ").length());

        bool readOk = true;
        while (in.readLineInto(&line))
        {
            if (line[0] == '#') // meta info
            {
                if (line.startsWith("#sensorId:"))
                    setSensorId(line.right(line.length()-QString("#sensorId:").length()));
                else if (line.startsWith("#failures:")) {
                    QString failureString = line.right(line.length()-QString("#failures:").length());
                    failureString=failureString.split(";").join("");
                    if (failureString.size() != MVector::size)
                    {
                        qWarning() << "Failure string not valid. Using empty failure string.";
                        failureString = "0000000000000000000000000000000000000000000000000000000000000000";
                    }
                    setFailures(failureString);
                }
                else if (line.startsWith("#functionalisation:"))
                {
                    QString rawString = line.right(line.length()-QString("#functionalisation:").length());
                    QStringList funcList = rawString.split(";");

                    for (int i=0; i<functionalisation.size(); i++)
                        functionalisation[i] = funcList[i].toInt();
                }
                else if (line.startsWith("#baseLevel:"))
                {
                    line = line.split(":")[1];
                    QStringList valueList = line.split(";");

                    uint timestamp = valueList[0].toUInt();
                    MVector baseLevel;
                    for (int i=0; i<MVector::size; i++)
                        baseLevel.array[i] = valueList[i+1].toDouble();

                    setBaseLevel(timestamp, baseLevel);
                }
                else    // comment
                    comment += line.right(line.length()-1) + "\n";
            } else  // data
            {
                QStringList query = line.split(";");

                // line has to contain vector and baselevel vector
                if (query.size() != MVector::size+1)
                {
                    QMessageBox::critical(static_cast<QWidget*>(this->parent()), "Error loading measurement data", "Data format is not compatible (len=" + QString::number(query.size()) + "):\n" + line);
                    readOk = false;
                    break;
                } else if (baseLevelMap.isEmpty())
                {
                    QMessageBox::critical(static_cast<QWidget*>(this->parent()), "Error loading measurement data", "No baseLevel in data");
                    readOk = false;
                    break;
                }
                // prepare vector
                uint timestamp = query[0].toUInt(&readOk);
                MVector vector;
                for (int i=0; i<MVector::size; i++)
                {
                    vector.array[i] = query[i+1].toDouble(&readOk);
                }
                addMeasurement(timestamp, vector, getBaseLevel(timestamp));
            }

            // catch errors reading files
            if (!readOk)
            {
                QMessageBox::critical(widget, "Error reading file", "Error in line:\n" + line);
                clear();
                clearSelection();
                return false;
            }
        }

        setComment(comment);
        dataChanged = false;

        // emit signal to replot graphs
        // DEBUG
//        emit dataSet(data);

//        // absolute data
//        QMap<uint, MVector> absoluteData;
//        for (auto ts : data.keys())
//        {
//            MVector baseLevelVector = getBaseLevel(ts);
//            MVector absoluteVector;

//            for (int i=0; i<MVector::size; i++)
//                absoluteVector.array[i] = data[ts].array[i] + baseLevelVector.array[i];

//            absoluteData[ts] = absoluteVector;
//        }

//        emit absoluteDataSet(absoluteData);

        return true;
    }
}

void MeasurementData::generateRandomWalk()
{
    clear();

    uint now = QDateTime::currentDateTime().toTime_t();
    int n = 100;

    MVector baseLevel;
    for (int i=0; i<n; i++)
    {
        MVector measurement;
        for(int j=0; j<64; j++)
        {
            if (i == 0)
                measurement.array[j] = 1500.0 + (300.0)*(rand()/(double)RAND_MAX-0.5);

            else
                measurement.array[j] = (1+data[now+i-1].array[j]/100.0) * baseLevel.array[j] + 100.0 * (rand()/(double)RAND_MAX-0.5) + j/64.0;
        }

        if (i == 0)
        {
            emit startTimestempSet(now);
            baseLevel = measurement;
            setBaseLevel(now-1, baseLevel);
        }
        addMeasurement(now+i, measurement);
    }

    setSensorId("Randomly generated data");
}

void MeasurementData::setSelection(int lower, int upper)
{
    // clear selectedData
    clearSelection();

    // selection deselected
    if (upper < lower)
    {
        emit selectionCleared();
        return;
    }

    MVector vector;

    qDebug() << "Selection requested: " << lower << ", " << upper;

    // single selection:
    if (lower == upper)
    {
        // get selected vector
        vector = data[lower];

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
            if (!endIterSet && timestamp >= upper)
            {
                endIter = data.find((timestamp));
                endIterSet = true;
                // increment endIter in order to point one element further than upper
                endIter++;
            }
        }

        // upper > timestamp of last vector
        if (!endIterSet)
            endIter = data.end();

        // no vector in the interval [lower; upper]
        if (!beginIterSet || (beginIter != data.end() && beginIter.key()>upper) || (endIter != data.end() && endIter.key()<lower))
        {
            clearSelection();
            return;
        }

        // calculate average vector
        vector = getSelectionVector(beginIter, endIter);

        // add selected vectors to seletedData
        for (auto iter=beginIter; iter != endIter; iter++)
            selectedData[iter.key()] = iter.value();
    }

    qDebug() << "Selection made: " << selectedData.firstKey() << ", " << selectedData.lastKey() << "\n" << vector.toString() << "\n";
    emit selectionVectorChanged(vector, sensorFailures);
    emit selectionMapChanged(selectedData);
}

const MVector MeasurementData::getSelectionVector(QMap<uint, MVector>::iterator begin, QMap<uint, MVector>::iterator end, uint endTimestamp, MultiMode mode)
{
    // init vector
    MVector vector;
    for (int i=0; i<MVector::size; i++)
        vector.array[i] = 0.0;

    if (mode == MultiMode::Average)
    {
    auto iter = begin;
    int n = 0;

    // iterate until end; if endTimestamp was set (!= 0) also check for endTimestamp
    while (iter != end && (endTimestamp==0 || iter.key() <= endTimestamp))
    {
        for (int i=0; i<MVector::size; i++)
            vector.array[i] += iter.value().array[i];

        iter++;
        n++;
    }


    for (int i=0; i<MVector::size; i++)
        vector.array[i] = vector.array[i] / n;
    }
    else
        Q_ASSERT ("Error: Invalid MultiMode." && false);

    return vector;
}

const MVector MeasurementData::getSelectionVector(MultiMode mode)
{
    // only average supported
    Q_ASSERT(mode == MultiMode::Average);

    auto selectionMap = getSelectionMap();

    MVector vector;
    // zero init
    for (int i=0; i<MVector::size; i++)
        vector.array[i] = 0.0;

    for (auto timestamp : selectionMap.keys())
    {
        // calculate average
        if (mode == MultiMode::Average)
            for (int i=0; i<MVector::size; i++)
                vector.array[i] += selectionMap[timestamp].array[i] / selectionMap.size();
    }

    return vector;
}

QString MeasurementData::sensorFailureString(std::array<bool, 64> failureBits)
{
    QString failureString;

    for (int hex=0; hex<MVector::size/4; hex++)
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

std::array<bool, MVector::size> MeasurementData::sensorFailureArray(QString failureString)
{
    std::array<bool, MVector::size> failureArray;
    for (int i=0; i<MVector::size; i++)
        failureArray[i] = 0;

    if (failureString == "None")
        return failureArray;

    for (int hex=0; hex < failureString.length(); hex++)
    {
        int failureInt = QString(failureString[failureString.length()-hex-1]).toInt(nullptr, 16);

        for (int bit=0; bit<4; bit++)
        {
            failureArray[4*hex+bit] |= failureInt & (1UL << bit);
        }
    }
}

void MeasurementData::setUserDefinedClassOfSelection(QString className, QString classBrief)
{
    for (auto timestamp : selectedData.keys())
    {
        // data and selectedData
        data[timestamp].userDefinedClass = aClass(className, classBrief);

        selectedData[timestamp].userDefinedClass = aClass(className, classBrief);
    }

    emit selectionMapChanged(selectedData);
}

void MeasurementData::setDetectedClassOfSelection(QString className, QString classBrief)
{
    for (auto timestamp : selectedData.keys())
    {
        // data and selectedData
        data[timestamp].detectedClass = aClass(className, classBrief);

        selectedData[timestamp].detectedClass = aClass(className, classBrief);
    }

    emit selectionMapChanged(selectedData);
}

void MeasurementData::addClass(aClass newClass)
{
    Q_ASSERT("Trying to add class that already exists!" && !classList.contains(newClass));

    classList.append(newClass);
}

void MeasurementData::removeClass(aClass oldClass)
{
    Q_ASSERT("Trying to remove class that does not exist!" && classList.contains(oldClass));

    int index = classList.indexOf(oldClass);
    classList.removeAt(index);
}

void MeasurementData::changeClass(aClass oldClass, aClass newClass)
{
    Q_ASSERT("Trying to change class that does not exist!" && classList.contains(oldClass));
    Q_ASSERT("Trying to change class to new class that already exists!" && !classList.contains(newClass));

    int index = classList.indexOf(oldClass);
    classList[index] = newClass;
}
