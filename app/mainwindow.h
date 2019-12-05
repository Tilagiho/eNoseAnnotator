#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QMainWindow>
#include "measurementdata.h"
#include "datasource.h"
#include "mvector.h"
#include <QCloseEvent>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionSave_Data_triggered();

    void on_actionsave_selection_triggered();

    void on_actionLoad_triggered();

    void on_lSplitter_splitterMoved(int pos, int index);

    void on_rSplitter_splitterMoved(int pos, int index);

    void on_actionSet_USB_Connection_triggered();

    void on_actionSettings_triggered();

    void on_actionStart_triggered();

    void on_actionStop_triggered();

    void on_actionReset_triggered();

    void on_actionClassify_selection_triggered();

    void on_actionSet_detected_class_of_selection_triggered();

    void on_actionSave_triggered();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;

    QLabel *statusTextLabel;
    QLabel *statusImageLabel;

    MeasurementData *mData = nullptr;
    DataSource *source = nullptr;

    void closeEvent (QCloseEvent *event);

    void createStatusBar();

    void clearData();

    void makeSourceConnections();

    void sensorConnected(QString sensorId);
    void stopMeas();
    void startMeas();
    void resetMeas();
};

#endif // MAINWINDOW_H
