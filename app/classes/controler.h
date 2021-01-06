#ifndef CONTROLER_H
#define CONTROLER_H

#include <QObject>

#include "../widgets/mainwindow.h"

#include "measurementdata.h"
#include "datasource.h"
#include "mvector.h"
#include "torchclassifier.h"
#include "classifier_definitions.h"

class ParseResult
{
public:
    ParseResult() {}

    QString filename;
    bool curveFit = false;
    int timeout = -1;
    int nCores = -1;
    int tOffset = 0;
    int tExposition = -1;
    int tRecovery = CVWIZ_DEFAULT_RECOVERY_TIME;

    QString toString()
    {
        QString resultString;
        resultString += "filename:\t" + filename + "\n";
        resultString += "curveFit:\t" + QString::number(curveFit) + "\n";
        resultString += "timeout:\t" + QString::number(timeout) + "\n";
        resultString += "nCores:\t" + QString::number(nCores) + "\n";

        return resultString;
    }
};

class Controler : public QObject
{
    Q_OBJECT
public:
    explicit Controler(QObject *parent = nullptr);
    ~Controler();

     MainWindow* getWindow() const;

     ParseResult getParseResult() const;

signals:

public slots:
     void initialize();
    void loadCLArguments();
    void loadAutosave();
    void loadSettings();
    void initSettings();

    void saveData();
    void saveData(bool forceDialog);
    void loadData();
    void loadData(QString filename);

    void setDataChanged(bool);

    void parseArguments();

private:
    MainWindow* w;

    QString autosaveName = "autosave.csv";
    QString autosavePath;
    uint autosaveIntervall = 1;             // in minutes
    QTimer autosaveTimer;

    MeasurementData *mData = nullptr;
    DataSource *source = nullptr;
    QThread* sourceThread = nullptr;
    TorchClassifier *classifier = nullptr;

    InputFunctionType inputFunctionType = InputFunctionType::medianAverage;
    ParseResult parseResult;

private slots:
    void clearData();

    bool dirIsWriteable(QDir dir);

    void saveSelection();

    void deleteAutosave();

    void saveDataDir();

    void selectFunctionalisation();

    void setGeneralSettings();

    void setSourceConnection();

    void makeSourceConnections();

    void startMeasurement();

    void stopMeasurement();

    void pauseMeasurement();

    void resetMeasurement();

    void reconnectMeasurement();

    void annotateGroundTruthOfSelection();

    void deleteGroundTruthOfSelection();

    void annotateDetectionOfSelection();


    void loadClassifier();

    void closeClassifier();

    void classifyMeasurement();

    void updateAutosave();

    void fitCurves();
};

#endif // CONTROLER_H
