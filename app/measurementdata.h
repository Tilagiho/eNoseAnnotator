#ifndef MEASUREMENTDATA_H
#define MEASUREMENTDATA_H

#include <QtCore>
#include "mvector.h"

#include <QtCore>
#include <QObject>
#include <QMap>


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
     * add relative vector + baseLevelVector to data
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

    /*
     * saves the content of map
     * opens QFileDialog in order to get the save path
     */
    bool saveData(QWidget* widget, QMap<uint, MVector> map);

    /*
     *  saves current selection
     * opens QFileDialog in order to get the save path
     */
    bool saveSelection(QWidget* widget);

    /*
     * loads data file
     * opens QFileDialog in order to get path to the data file
     */
    bool loadData(QWidget* widget);


    /*
     *  generates a random walk dataset
     */
    void generateRandomWalk();

    /*
     * calculates average of vectors in range from iterator begin until, but not including, iterator end
     * if endTimestamp is set: additionally end if timestamp of current vector > endTimestamp
     */
    static const MVector getSelectionVector(QMap<uint, MVector>::iterator begin, QMap<uint, MVector>::iterator end, uint endTimestamp=0, MultiMode mode=MultiMode::Average);

    const MVector getSelectionVector(MultiMode mode=MultiMode::Average);


    QString getSensorId() const;

    std::array<bool, 64> getSensorFailures() const;
    void setSensorFailures(const std::array<bool, 64> &value);

    static QString sensorFailureString(std::array<bool, 64>);
    static std::array<bool, 64> sensorFailureArray(QString);

    /*
     * returns true if data was changed since last save/ load action
     */
    bool changed() const;

    /*
     * returns baselevel at timestamp
     */
    MVector getBaseLevel (uint timestamp);

    std::array<int, MVector::size> getFunctionalities() const;
    void setFunctionalities(const std::array<int, MVector::size> &value);

    bool getSaveRawInput() const;
    void setSaveRawInput(bool value);

    /*
     * set the user defined class of the current selection
     */
    void setUserDefinedClassOfSelection(QString className, QString classBrief);

    /*
     * set the detected class of the current selection
     */
    void setDetectedClassOfSelection(QString className, QString classBrief);



    QList<aClass> getClassList() const;

public slots:
    /*
     * clears selectedData and adds all vectors with timestamp between lower and upper to selectedData
     */
    void setSelection(int lower, int upper);

    void setComment(QString comment);
    void setFailures(std::array<bool, 64>);
    void setFailures(QString failureString);
    void setSensorId(QString sensorId);
    void setBaseLevel(uint timestamp, MVector baseLevel);
    void addClass(aClass newClass);
    void removeClass(aClass oldClass);
    void changeClass(aClass oldClass, aClass newClass);

signals:
    void selectionVectorChanged(MVector vector, std::array<bool, MVector::size> sensorFailures);  // emits new vector when dataSelected is changed
    void selectionMapChanged(QMap<uint, MVector> selectionMap);
    void labelsUpdated(QMap<uint, MVector> updatedVectors);
    void selectionCleared();
    void dataReset();   // emitted when data is reset
    void dataAdded(MVector vector, uint timestamp, bool yRescale);
    void absoluteDataAdded(MVector vector, uint timestamp, bool yRescale);
    void dataSet(QMap<uint, MVector>);
    void absoluteDataSet(QMap<uint, MVector>);
    void sensorIdSet(QString sensorId);
    void startTimestempSet(uint timestamp);
    void commentSet(QString comment);
    void sensorFailuresSet(std::array<bool, 64>);

private:
    QMap<uint, MVector> data;  // map containing vectors of measurements with timestamps as keys
    QMap<uint, MVector> selectedData;
    QMap<uint, MVector> baseLevelMap;
    std::array<int, MVector::size> functionalisation;
    bool dataChanged = false;
    QString dataComment = "";
    QString sensorId = "";
    std::array<bool, 64> sensorFailures;
    bool saveRawInput = false;      // if true: save absolute values in "raw_input_" + timestamp
    QList<aClass> classList;

    QString version = "v0.1";
};

#endif // MEASUREMENTDATA_H
