#ifndef MEASUREMENTDATA_H
#define MEASUREMENTDATA_H

#include <QtCore>
#include <QObject>
#include <QMap>

#include "mvector.h"
#include "classifier_definitions.h"
#include "leastsquaresfitter.h"
#include "functionalisation.h"
#include "defaultSettings.h"

class MeasurementData : public QObject
{
    Q_OBJECT

public:
    explicit MeasurementData(QObject *parent, size_t nChannels = MVector::nChannels);
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
    QMap<uint, RelativeMVector> getRelativeData();

    QMap<uint, RelativeMVector> getFuncData();

    /*
     * returns absolute data in a map<timestamp, vector>
     */
    const QMap<uint, AbsoluteMVector>& getAbsoluteData();

    /*
     * returns current selection in a map<timestamp, vector>
     */
    const QMap<uint, AbsoluteMVector>& getSelectionMap();

    QString getComment();
    QString getFailureString();

    void setData (QMap<uint, AbsoluteMVector> absoluteData, QMap<uint, AbsoluteMVector> baseVectors);

    void setClasslist(QList<aClass> classList);

    void setSensorAttributes(QSet<QString> sensorAttributes);

    void setSaveFilename(QString saveFilename);

    /*
     * returns vector stored at timestamp
     * warning: runtime error if timestamp not in data! use contains() to check before
     */
    MVector getMeasurement(uint timestamp);

    uint getStartTimestamp();

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
    bool saveAverageSelectionVector(QString filename, bool saveAbsolute);
    bool saveAverageSelectionFuncVector(QString filename, bool saveAbsolute);

//    /*
//     * calculates average of vectors in range from iterator begin until, but not including, iterator end
//     * if endTimestamp is set: additionally end if timestamp of current vector > endTimestamp
//     */
//    static const MVector getSelectionVector(QMap<uint, MVector>::iterator begin, QMap<uint, MVector>::iterator end, uint endTimestamp=0, MultiMode mode=MultiMode::Average);

    void copyFrom(MeasurementData* otherMData);

    const AbsoluteMVector getAbsoluteSelectionVector(MVector *stdDevVector=nullptr, MultiMode mode=MultiMode::Average);
    const RelativeMVector getRelativeSelectionVector(MVector *stdDevVector=nullptr, MultiMode mode=MultiMode::Average);

    QString getSensorId() const;

    std::vector<bool> getSensorFailures() const;

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
    AbsoluteMVector* getBaseVector (uint timestamp);

    Functionalisation getFunctionalisation() const;
    void setFunctionalisation(const Functionalisation &value);

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

    QMap<uint, AbsoluteMVector> getBaseLevelMap() const;

    void setInputFunctionType(const InputFunctionType &value);

    uint getNextTimestamp (uint timestamp);

    uint getPreviousTimestamp (uint timestamp);

    double getLowerLimit() const;

    double getUpperLimit() const;

    bool getUseLimits() const;

    size_t nChannels() const;

public slots:
    /*
     * clears selectedData and adds all vectors with timestamp between lower and upper to selectedData
     */
    void setSelection(uint lower, uint upper);

    void setComment(QString comment);
    void setSensorFailures(const std::vector<bool> &);
    void setSensorFailures(const QString failureString);
    void setSensorId(QString sensorId);
    void setBaseVector(uint timestamp, AbsoluteMVector baseVector);
    void addClass(aClass newClass);
    void removeClass(aClass oldClass);
    void changeClass(aClass oldClass, aClass newClass);
    void setFuncName(QString);
    void addAttributes(QSet<QString> attributes);
    void deleteAttributes(QSet<QString> attributes);
    void renameAttribute(QString oldName, QString newName);
    void resetNChannels();

    /*
     * add absolute vector with timestamp to data
     */
    void addVector(uint timestamp, AbsoluteMVector vector);

    /*
     * add absolute vector + baseLevelVector to data
     */
    void addVector(uint timestamp, AbsoluteMVector vector, AbsoluteMVector baseLevelVector);

    void checkLimits (const AbsoluteMVector &vector);
    void checkLimits ();

    void setLimits(double lowerLimit, double upperLimit, bool useLimits);

signals:
    void selectionVectorChanged(const AbsoluteMVector &vector, const MVector &stdDevVector, const std::vector<bool> &sensorFailures, const Functionalisation &functionalisation);  // emits new vector when dataSelected is changed
    void selectionMapChanged(QMap<uint, MVector> selectionMap);
    void annotationsChanged(const QMap<uint, Annotation> annotations, bool isUserAnnotation);

    // emitted when selectionData was cleared
    void selectionCleared();

    // emitted when LineGraphWidget should clear its selection
    void lgClearSelection();

    void dataReset();   // emitted when data is reset
    void relativeVectorAdded(MVector vector, uint timestamp, Functionalisation functionalisation , std::vector<bool> sensorFailures, bool yRescale);
    void vectorAdded(uint timestamp, AbsoluteMVector vector, Functionalisation functionalisation , std::vector<bool> sensorFailures, bool yRescale);
    void dataSet(const QMap<uint, AbsoluteMVector> &data, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures);

    //    void dataSet(QMap<uint, MVector> data, Functionalisation functionalisation , std::vector<bool> sensorFailures);
    void absoluteDataSet(QMap<uint, MVector>);
    void sensorIdSet(QString sensorId);
    void startTimestempSet(uint timestamp);
    void commentSet(QString comment);
    void sensorFailuresSet(const QMap<uint, AbsoluteMVector> &data, Functionalisation &functionalisation, const std::vector<bool> &sensorFailures);

    void dataChangedSet(bool);

    void saveFilenameSet();

    void functionalisationChanged();

private:
    QMap<uint, AbsoluteMVector> data;  // map containing vectors of measurements with timestamps as keys
    QMap<uint, AbsoluteMVector> selectedData;
    QMap<uint, AbsoluteMVector> baseVectorMap;

    Functionalisation functionalisation;
    std::vector<bool> sensorFailures;

    bool dataChanged = false;
    QString dataComment = "";
    QString sensorId = "";
    QList<aClass> classList;

    QString savefileFormatVersion = "1.0";

    QString saveFilename = "./data/";

    QSet<QString> sensorAttributes;

    InputFunctionType inputFunctionType = InputFunctionType::medianAverage;

    bool replotStatus = true;

//    int nChannels = MVector::nChannels;

    double lowerLimit = DEFAULT_LOWER_LIMIT;
    double upperLimit = DEFAULT_UPPER_LIMIT;
    bool useLimits = DEFAULT_USE_LIMITS;
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
    Functionalisation functionalistation;
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
    QMap<size_t, size_t> resistanceIndexes;
    int t_index = -1;
    uint start_time = 0;
};

#endif // MEASUREMENTDATA_H
