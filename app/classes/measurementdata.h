#ifndef MEASUREMENTDATA_H
#define MEASUREMENTDATA_H

#include <QtCore>
#include <QObject>
#include <QMap>

#include "mvector.h"
#include "classifier_definitions.h"

class MeasurementData : public QObject
{
    Q_OBJECT

public:
    explicit MeasurementData(QObject *parent, int nChannels = MVector::nChannels);
    /*
     * enumeration MultiMode can be used to define how the result of a selection of multiple vectors should be calculated
     */
    ~MeasurementData();

    enum class MultiMode {
        Average // calculate average of selected vectors
    };

    /*
     *  returns list of all MultiModes
     */
    static QList<MultiMode> getMultiModeList()
    {
        return QList<MultiMode>{MultiMode::Average};
    }

    /*
     * returns QString associated with mode
     */
    static QString multiModeToQString(MultiMode mode)
    {
        switch (mode)
        {
        case MultiMode::Average:
            return "Average";
        default:
            Q_ASSERT(false && "MultiMode not defined");
        }
    }

    /*
     * returns mode associated with modeString
     */
    static MultiMode qStringToMultiMode(QString modeString)
    {
        if (modeString == "Average")
            return MultiMode::Average;
        else
            Q_ASSERT(false && "MultiMode not defined");
    }

    /*
     * returns relative data in a map<timestamp, vector>
     */
    const QMap<uint, MVector> getRelativeData();

    const QMap<uint, MVector> getFuncData();

    /*
     * returns absolute data in a map<timestamp, vector>
     */
    const QMap<uint, MVector> getAbsoluteData();

    /*
     * returns current selection in a map<timestamp, vector>
     */
    const QMap<uint, MVector> getSelectionMap();

    int getNFuncs() const;

    QString getComment();
    QString getFailureString();

    /*
     * add absolute vector with timestamp to data
     */
    void addMeasurement(uint timestamp, MVector vector);

    /*
     * add absolute vector + baseLevelVector to data
     */
    void addMeasurement(uint timestamp, MVector vector, MVector baseLevelVector);

    void setData (QMap<uint, MVector> absoluteData, QMap<uint, MVector> baseVectors);

    void setClasslist(QList<aClass> classList);

    void setSensorAttributes(QSet<QString> sensorAttributes);

    void setSaveFilename(QString saveFilename);

    /*
     * returns vector stored at timestamp
     * warning: runtime error if timestamp not in data! use contains() to check before
     */
    MVector getMeasurement(uint timestamp);

    bool contains(uint timestamp);

    /*
     * clears all data and info except sensor failures
     */
    void clear();

    /*
     * clears current selection
     */
    void clearSelection();

    /*
     * saves data
     * opens QFileDialog in order to get the save path
     */
    bool saveData(QString filename);


    /*
     * saves the content of map
     * opens QFileDialog in order to get the save path
     */
    bool saveData(QString filename, QMap<uint, MVector> map);

    /*
     *  saves current selection
     * opens QFileDialog in order to get the save path
     */
    bool saveSelection();
    bool saveSelection(QString filename);
    bool saveAverageSelectionMeasVector(QString filename);
    bool saveAverageSelectionFuncVector(QString filename);

//    /*
//     * calculates average of vectors in range from iterator begin until, but not including, iterator end
//     * if endTimestamp is set: additionally end if timestamp of current vector > endTimestamp
//     */
//    static const MVector getSelectionVector(QMap<uint, MVector>::iterator begin, QMap<uint, MVector>::iterator end, uint endTimestamp=0, MultiMode mode=MultiMode::Average);

    void copyFrom(MeasurementData* otherMData);

    const MVector getSelectionVector(MultiMode mode=MultiMode::Average);


    QString getSensorId() const;

    std::vector<bool> getSensorFailures() const;
    void setSensorFailures(const std::vector<bool> &value);

    static QString sensorFailureString(std::vector<bool>);
    static std::vector<bool> sensorFailureArray(QString);

    /*
     * returns true if data was changed since last save/ load action
     */
    bool isChanged() const;

    void setDataChanged(bool isChanged);

    /*
     * returns baselevel at timestamp
     */
    MVector getBaseLevel (uint timestamp);

    std::vector<int> getFunctionalisation() const;
    void setFunctionalisation(const std::vector<int> &value);

    static QMap<int, int> getFuncMap(const std::vector<int> &funcs, const std::vector<bool> sensorFailures);

    static QMap<int, int> getFuncMap();

    bool getSaveRawInput() const;
    void setSaveRawInput(bool value);

    /*
     * set the user defined class of the current selection
     */
    void setUserAnnotationOfSelection(Annotation annotation);


    /*
     * set the user defined class at timestamp
     */
    void setUserAnnotation(Annotation annotation, uint timestamp);

    /*
     * set the detected class of the current selection
     */
    void setDetectedAnnotationOfSelection(Annotation annotation);

    /*
     * set the detected defined class at timestamp
     */
    void setDetectedAnnotation(Annotation annotation, uint timestamp);

    static QString getTimestampStringFromUInt(uint timestamp);
    static uint getTimestampUIntfromString (QString string);


    QList<aClass> getClassList() const;

    QString getSaveFilename() const;

    QSet<QString> getSensorAttributes() const;

    QMap<uint, MVector> getBaseLevelMap() const;

    QString funcName = "None";
    static std::vector<int> functionalisation;
    static std::vector<bool> sensorFailures;


    void setInputFunctionType(const InputFunctionType &value);

public slots:
    /*
     * clears selectedData and adds all vectors with timestamp between lower and upper to selectedData
     */
    void setSelection(int lower, int upper);

    void setComment(QString comment);
    void setFailures(std::vector<bool>);
    void setFailures(QString failureString);
    void setSensorId(QString sensorId);
    void setBaseLevel(uint timestamp, MVector baseLevel);
    void addClass(aClass newClass);
    void removeClass(aClass oldClass);
    void changeClass(aClass oldClass, aClass newClass);
    void setFuncName(QString);
    void addAttributes(QSet<QString> attributes);
    void deleteAttributes(QSet<QString> attributes);
    void renameAttribute(QString oldName, QString newName);
    void resetNChannels();

signals:
    void selectionVectorChanged(MVector vector, std::vector<bool> sensorFailures, std::vector<int>);  // emits new vector when dataSelected is changed
    void selectionMapChanged(QMap<uint, MVector> selectionMap);
    void labelsUpdated(QMap<uint, MVector> updatedVectors);

    // emitted when selectionData was cleared
    void selectionCleared();

    // emitted when LineGraphWidget should clear its selection
    void lgClearSelection();

    void dataReset();   // emitted when data is reset
    void dataAdded(MVector vector, uint timestamp, bool yRescale);
    void absoluteDataAdded(MVector vector, uint timestamp, bool yRescale);
    void dataSet(QMap<uint, MVector>);
    void absoluteDataSet(QMap<uint, MVector>);
    void sensorIdSet(QString sensorId);
    void startTimestempSet(uint timestamp);
    void commentSet(QString comment);
    void sensorFailuresSet(std::vector<bool>);

    // emitted when replotStatus in LinegraphWidgets should be set
    void setReplotStatus(bool status);

    void dataChangedSet(bool);

    void saveFilenameSet();

    void functionalisationChanged();

    void funcNameSet(QString);

private:
    QMap<uint, MVector> data;  // map containing vectors of measurements with timestamps as keys
    QMap<uint, MVector> selectedData;
    QMap<uint, MVector> baseLevelMap;

    bool dataChanged = false;
    QString dataComment = "";
    QString sensorId = "";
    QList<aClass> classList;

    QString savefileFormatVersion = "1.0";

    QString saveFilename = "./data/";

    QSet<QString> sensorAttributes;

    InputFunctionType inputFunctionType = InputFunctionType::medianAverage;

//    int nChannels = MVector::nChannels;
};

/*!
 * \brief The fileParser class parses file & returns Measurementdata with the information contained within it.
 */
class FileReader : public QObject
{
    Q_OBJECT

public:
    enum FileReaderType {General, Annotator, Leif};

    FileReader(QString filePath, QObject *parent=nullptr);
    virtual ~FileReader();

    MeasurementData* getMeasurementData();
    FileReader* getSpecificReader();

    virtual void readFile(){};

    virtual FileReaderType getType();

signals:
    void resetNChannels(uint nChannels);

private:


protected:
    MeasurementData* data;
    QFile file;
    QTextStream in;
    int lineCount = -1;
};

class AnnotatorFileReader : public FileReader
{
public:
    AnnotatorFileReader(QString filePath);

    FileReaderType getType() override;

    void readFile() override;

private:
    void parseHeader(QString line);
    void parseValues(QString line);

    QString formatVersion;

    uint timestampIndex = 0;
    int userAnnotationIndex = -1;
    int detectedAnnotationIndex = -1;

    QMap<uint, uint> resistanceIndexMap;
    QMap<QString, uint> sensorAttributeMap;

    // meas meta attributes
    QString failureString;
    std::vector<int> functionalistation;
    QMap<uint, MVector> baseLevelMap;
};

class LeifFileReader : public FileReader
{
public:
    LeifFileReader(QString filePath);

    FileReaderType getType() override;

    void readFile() override;

private:
    void parseHeader(QString line);
    void parseFuncs(QString line);
    void parseValues(QString line);

    QMap<QString, int> sensorAttributeIndexMap;
    QMap<int, int> resistanceIndexes;
    int t_index = -1;
    uint start_time = 0;

    std::vector<int> functionalistation;
};

#endif // MEASUREMENTDATA_H
