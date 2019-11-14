#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QMainWindow>
#include "measurementdata.h"
#include "annotationdataset_old.h"
#include "editannotationdatawindow.h"
#include "usbdatasource.h"
#include "mvector.h"

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

    void on_actionFunctionalitization_triggered();

    void on_actionSaveAnnotation_triggered();

    void on_actionOpenAnnotation_triggered();

    void on_actionSettings_triggered();

    void on_actionStart_triggered();

    void on_actionStop_triggered();

    void on_actionReset_triggered();

private:
    Ui::MainWindow *ui;

    MeasurementData *mData = nullptr;
    AnnotationDatasetModel *aDataModel = nullptr;
    USBDataSource *usbSource = nullptr;

    editAnnotationDataWindow* annotationDataWindow = nullptr;
};
#endif // MAINWINDOW_H
