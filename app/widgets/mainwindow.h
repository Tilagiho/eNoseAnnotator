#ifndef MAINWINDOW_H
#define MAINWINDOW_H
// for windows build
#define DLPACK_EXPORTS

#include <QtCore>
#include <QMainWindow>
#include <QCloseEvent>
#include <QLabel>

#include "linegraphwidget.h"
#include "bargraphwidget.h"
#include "infowidget.h"
#include "classifierwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool getConverterRunning() const;

signals:
    void setConnectionRequested();
    void startRequested();
    void stopRequested();
    void pauseRequested();
    void resetRequested();
    void reconnectRequested();

    void selectionGroundTruthAnnotationRequested();
    void selectionDetectionAnnotationRequested();
    void deleteGroundTruthAnnotationRequested();

    void loadMeasurementRequested();
    void saveDataRequested();
    void saveDataAsRequested();
    void saveSelectionRequested();

    void generalSettingsRequested();

    void loadClassifierRequested();
    void classifyMeasurementRequested();

    void fitCurvesRequested();

    void sensorFailuresSet(const std::vector<bool> &sensorFailures);
    void functionalisationSet(const Functionalisation &functionalisation);

    void sensorFailureDialogRequested();
    void functionalisationDialogRequested();

    void selectionMade(uint min, uint max);
    void selectionCleared();

    void startTimestempSet(uint);
    void commentSet(QString);
    void sensorIdSet(QString);

public slots:

    void addVector(uint timestamp, AbsoluteMVector absoluteVector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures);
    void setData(const QMap<uint, AbsoluteMVector> &data, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures);
    void clearGraphs();

    void setStatus(DataSource::Status newStatus);

    void sensorConnected(QString sensorId);

    void setFunctionalisation(const QMap<uint, AbsoluteMVector> &data, Functionalisation &functionalisation, std::vector<bool> &sensorFailures);

    void setSensorFailures(const QMap<uint, AbsoluteMVector> &data, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures);

    void openSensorFailuresDialog(const std::vector<bool> &sensorFailures);

    void setAbsoluteLimits(double lowerLimit, double upperLimit, bool useLimits);

    void setDataChanged(bool dataIsChanged, QString filename);

    void setClassifier(QString name, QStringList classNames, bool isInputAbsolute, QString presetName);

    void closeClassifier();

    void changeAnnotations( const QMap<uint, Annotation> annotations, bool isUserAnnotation );

    void setSelectionVector ( const AbsoluteMVector &vector, const std::vector<bool> &sensorFailures, const Functionalisation &functionalisation );

    void clearSelectionVector ();

    void setTitle( QString title, bool dataChanged );

private slots:
    void on_actionSave_Data_As_triggered();

    void on_actionsave_selection_triggered();

    void on_actionLoad_triggered();

    void on_actionSet_USB_Connection_triggered();

    void on_actionSettings_triggered();

    void on_actionStart_triggered();

    void on_actionStop_triggered();

    void on_actionReset_triggered();

    void on_actionAnnotate_selection_triggered();

    void on_actionSet_detected_class_of_selection_triggered();

    void on_actionSave_triggered();

    void on_actionAbout_triggered();

    void on_actionLoadClassifier_triggered();

    void on_actionDelete_Annotation_triggered();

    void on_actionReconnect_triggered();

    void on_actionClassify_measurement_triggered();

    void on_actionLive_classifcation_triggered(bool checked);

    void on_actionCloseClassifier_triggered();

    void on_actionConverter_triggered();

    void on_actionFit_curve_triggered();

    void redrawFuncGraph(const QMap<uint, AbsoluteMVector> &data, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures);

    void setSelectionActionsEnabled(bool selectionMade);

private:
    Ui::MainWindow *ui;

    FuncLineGraphWidget* funcLineGraph;
    AbsoluteLineGraphWidget* absLineGraph;
    RelativeLineGraphWidget* relLineGraph;
    RelVecBarGraphWidget* vectorBarGraph;
    FuncBarGraphWidget* funcBarGraph;
    InfoWidget* measInfoWidget;
    ClassifierWidget* classifierWidget;

    QList<QDockWidget*> leftDocks;
    QList<QDockWidget*> rightDocks;

    QLabel *statusTextLabel;
    QLabel *statusImageLabel;

    bool dataIsChanged = false;
    bool converterRunning = false;

    void closeEvent (QCloseEvent *event);

    void createDockWidgets();

    void createStatusBar();

    void makeSourceConnections();

    void updateFuncGraph();

    void connectFLGraph();

    void setIsLiveClassificationState(bool isLive);

    void saveLineGraph(LineGraphWidget* graph);

    void saveBarGraph(AbstractBarGraphWidget* graph);

    void resetNChannels(uint newNChannels);

protected:
  bool eventFilter(QObject *obj, QEvent *event);
};

#endif // MAINWINDOW_H
