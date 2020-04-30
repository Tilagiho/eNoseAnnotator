#ifndef MEASUREMENTDATA_H
#define MEASUREMENTDATA_H

#include <QtCore>
#include <QObject>
#include <QMap>

#include "mvector.h"

class MeasurementData : public QObject
{
    Q_OBJECT

public:
    explicit MeasurementData(QObject *parent = nullptr);
    /*
     * enumeration MultiMode can be used to define how the result of a selection of multiple vectors should be calculated
     */
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
    bool saveData(QWidget* widget);
    bool saveData(QWidget* widget, QString filename);


    /*
     * saves the content of map
     * opens QFileDialog in order to get the save path
     */
    bool saveData(QWidget* widget, QMap<uint, MVector> map);
    bool saveData(QWidget* widget, QString filename, QMap<uint, MVector> map);

    /*
     *  saves current selection
     * opens QFileDialog in order to get the save path
     */
    bool saveSelection(QWidget* widget);
    bool saveSelection(QWidget* widget, QString filename);
    bool saveAverageSelectionMeasVector(QWidget* widget, QString filename);
    bool saveAverageSelectionFuncVector(QWidget* widget, QString filename);


    /*
     * loads data file
     * opens QFileDialog in order to get path to the data file
     */
    bool loadData(QWidget* widget);

//    /*
//     * calculates average of vectors in range from iterator begin until, but not including, iterator end
//     * if endTimestamp is set: additionally end if timestamp of current vector > endTimestamp
//     */
//    static const MVector getSelectionVector(QMap<uint, MVector>::iterator begin, QMap<uint, MVector>::iterator end, uint endTimestamp=0, MultiMode mode=MultiMode::Average);

    const MVector getSelectionVector(MultiMode mode=MultiMode::Average);


    QString getSensorId() const;

    std::vector<bool> getSensorFailures() const;
    void setSensorFailures(const std::vector<bool> &value);

    static QString sensorFailureString(std::vector<bool>);
    static std::vector<bool> sensorFailureArray(QString);

    /*
     * returns true if data was changed since last save/ load action
     */
    bool changed() const;

    void setDataChanged(bool changed);

    /*
     * returns baselevel at timestamp
     */
    MVector getBaseLevel (uint timestamp);

    std::vector<int> getFunctionalities() const;
    void setFunctionalities(const std::vector<int> &value);

    static QMap<int, int> getFuncMap(const std::vector<int> &funcs, const std::vector<bool> sensorFailures);

    QMap<int, int> getFuncMap();

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

    static QString funcName;
    static std::vector<int> functionalisation;

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

    void functionalisationChanged();

    void funcNameSet(QString);

private:
    QMap<uint, MVector> data;  // map containing vectors of measurements with timestamps as keys
    QMap<uint, MVector> selectedData;
    QMap<uint, MVector> baseLevelMap;
    bool dataChanged = false;
    QString dataComment = "";
    QString sensorId = "";
    std::vector<bool> sensorFailures;
    QList<aClass> classList;

    QString savefileFormatVersion = "1.0";

    QString saveFilename = "./data/";

    QList<QString> sensorAttributes;

    /*
     * extracts measurement data from line
     */
    bool getData(QString line);

    /*
     * extracts meta data from line
     * meta data lines always start with '#'
     */
    bool getMetaData(QString line);


};

#endif // MEASUREMENTDATA_H
