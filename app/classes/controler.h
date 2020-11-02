#ifndef CONTROLER_H
#define CONTROLER_H

#include <QObject>

#include "../widgets/mainwindow.h"

#include "measurementdata.h"
#include "datasource.h"
#include "mvector.h"
#include "torchclassifier.h"
#include "classifier_definitions.h"

class Controler : public QObject
{
    Q_OBJECT
public:
    explicit Controler(QObject *parent = nullptr);
    ~Controler();

     MainWindow* getWindow() const;

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
