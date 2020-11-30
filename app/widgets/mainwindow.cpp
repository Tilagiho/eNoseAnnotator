#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "functionalisationdialog.h"
#include "generalsettings.h"
#include "classselector.h"
#include "linegraphwidget.h"
#include "sourcedialog.h"
#include "convertwizard.h"
#include "setsensorfailuresdialog.h"
#include "curvefitwizard.h"

#include <QMetaObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/icons/icon"));

    // init graph widgets
    createDockWidgets();

    // init statusbar
    statusTextLabel = new QLabel(statusBar());
    statusImageLabel = new QLabel(statusBar());
    statusTextLabel->setText("Sensor status: Not connected ");
    statusImageLabel->setPixmap(QPixmap(":/icons/disconnected"));
    statusImageLabel->setScaledContents(true);
    statusImageLabel->setMaximumSize(16,16);
    statusBar()->addPermanentWidget(statusTextLabel);
    statusBar()->addPermanentWidget(statusImageLabel);

    // prepare menubar
    ui->actionStart->setEnabled(false);
    ui->actionReset->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->actionReconnect->setEnabled(false);

    ui->actionAnnotate_selection->setEnabled(false);
    ui->actionDelete_Annotation->setEnabled(false);

    ui->actionClassify_measurement->setEnabled(false);
    ui->actionLive_classifcation->setChecked(true);

    ui->actionSet_detected_class_of_selection->setEnabled(false);

    ui->actionCloseClassifier->setEnabled(false);

    ui->actionFit_curve->setEnabled(false);

    // user can set detected class manually in debug mode
    #ifdef QT_NO_DEBUG
    ui->actionSet_detected_class_of_selection->setVisible(false);
    #endif    

    //                  //
    //  connections     //
    //                  //
    connect (this, &MainWindow::commentSet, measInfoWidget, &InfoWidget::setComment);
    connect (this, &MainWindow::sensorIdSet, measInfoWidget, &InfoWidget::setSensorId);


    // this->setStyleSheet("QSplitter::handle{background: black;}"); // make splitter visible

    // connections:
    // connect funcLineGraph, lGraph & absLGraph

//    connect(mData, &MeasurementData::selectionVectorChanged, this, [this](MVector selectionVector){
//        if (classifier != nullptr)
//        {
//            auto funcVector = selectionVector.getFuncVector(mData->getFunctionalisation(), mData->getSensorFailures(), inputFunctionType);

//            try {
//                Annotation annotation = classifier->getAnnotation(funcVector.getVector());
//                classifierWidget->setAnnotation(annotation);
//                classifierWidget->setInfoString("Class of selection");
//                classifierWidget->isSelectionAnnotation = true;
//            } catch (std::invalid_argument& e) {
//                QString error_message = e.what() + QString("\nDo you want to close the classifier?");
//                QMessageBox::StandardButton answer = QMessageBox::question(this, "Classifier error", error_message);
//                if (answer == QMessageBox::StandardButton::Yes)
//                {
//                    closeClassifier();
//                }
//            }
//        }
//    }); // classify selection
//    connect(mData, &MeasurementData::selectionCleared, classifierWidget, &ClassifierWidget::clearAnnotation); // clear selection classification

//    connect(mData, &MeasurementData::labelsUpdated, funcLineGraph, &LineGraphWidget::labelSelection); // draw selection and classes
//    connect(mData, &MeasurementData::labelsUpdated, absLineGraph, &LineGraphWidget::labelSelection); // draw selection and classes
//    connect(mData, &MeasurementData::labelsUpdated, relLineGraph, &LineGraphWidget::labelSelection); // draw selection and classes

//    connect(mData, &MeasurementData::selectionVectorChanged, this, [this](MVector, std::vector<bool>){
//        ui->actionAnnotate_selection->setEnabled(true);
//        ui->actionDelete_Annotation->setEnabled(true);
//        ui->actionSet_detected_class_of_selection->setEnabled(true);
//    }); // show classification actions
//    connect(mData, &MeasurementData::selectionCleared, this, [this](){
//        ui->actionAnnotate_selection->setEnabled(false);
//        ui->actionDelete_Annotation->setEnabled(false);
//        ui->actionSet_detected_class_of_selection->setEnabled(false);
//    }); // hide classification actions

//    // reset graphs
//    connect(mData, &MeasurementData::dataReset, this, [this]{
//        funcLineGraph->clearGraph();
//        relLineGraph->clearGraph();
//        absLineGraph->clearGraph();
//        vectorBarGraph->clearBars();
//        funcBarGraph->clearBars();
//    }); // clear all graphs when data is reset

//    // new data
//    connect(mData, &MeasurementData::dataAdded, this, [this](MVector vector, uint timestamp, std::vector<int> functionalisation, std::vector<bool> sensorFailures, bool yRescale){
//        // calculate funcVector, if necessary
//        bool calcFuncVector = false;
//        for (int func : mData->getFunctionalisation())
//        {
//            if (func != 0)
//            {
//                calcFuncVector = true;
//                break;
//            }
//        }
//        if (!calcFuncVector)
//            funcLineGraph->addMeasurement(vector, timestamp, functionalisation, sensorFailures, yRescale);
//        else
//        {
//            MVector funcVector = vector.getFuncVector(mData->getFunctionalisation(), mData->getSensorFailures(), inputFunctionType);
//            funcLineGraph->addMeasurement(funcVector, timestamp, functionalisation, sensorFailures, yRescale);
//        }
//    });    // add new data to func line graph
//    connect(mData, &MeasurementData::absoluteDataAdded, absLineGraph, &LineGraphWidget::addMeasurement); // add new absolute measruement
////    connect(mData, &MeasurementData::dataSet, relLineGraph, &LineGraphWidget::setData);     // set loaded data in lGraph
////    connect(mData, &MeasurementData::dataSet, this, [this](QMap<uint, MVector> data, std::vector<int> functionalisation, std::vector<bool> sensorFailures){
////        auto funcs = mData->getFunctionalisation();
////        auto failures = mData->getSensorFailures();

////        // add recalculated functionalitisation averages to cleared funcLineGraph
////        QMap<uint, MVector> funcData;
////        for (int timestamp : data.keys())
////            funcData[timestamp] = data[timestamp].getFuncVector(funcs, failures, inputFunctionType);

////        funcLineGraph->setData(funcData, functionalisation, sensorFailures);
////    });     // set loaded data in lGraph

//    connect(mData, &MeasurementData::setReplotStatus, funcLineGraph, &LineGraphWidget::setReplotStatus);   // replotStatus
//    connect(mData, &MeasurementData::setReplotStatus, relLineGraph, &LineGraphWidget::setReplotStatus);   // replotStatus
//    connect(mData, &MeasurementData::setReplotStatus, absLineGraph, &LineGraphWidget::setReplotStatus);   // replotStatus
//    connect(mData, &MeasurementData::dataAdded, this, [this](MVector vector, uint timestamp, std::vector<int> functionalisation, std::vector<bool> sensorFailures, bool){
//        // no classifier loaded or no live measurement running
//        // -> don't classify
//        bool measRunning = source != nullptr && source->status() == DataSource::Status::RECEIVING_DATA;
//        bool isLiveClassification = classifierWidget->getIsLive();
//        if (classifier == nullptr || !measRunning || !isLiveClassification)
//            return;

//        // get annotation from the classifier
//        auto funcVector = vector.getFuncVector(functionalisation, sensorFailures, inputFunctionType);

//        try {
//            Annotation annotation = classifier->getAnnotation(funcVector.getVector());

//            // set annotation
//            mData->setDetectedAnnotation(annotation, timestamp);

//            // don't show in classifier widget if selection is made
//            if (!classifierWidget->isSelectionAnnotation)
//            {
//                classifierWidget->setAnnotation(annotation);
//                classifierWidget->setInfoString("Live classification is running...");
//            }
//        } catch (std::invalid_argument& e) {
//            QString error_message = e.what() + QString("\nDo you want to close the classifier?");
//            QMessageBox::StandardButton answer = QMessageBox::question(this, "Classifier error", error_message);
//            if (answer == QMessageBox::StandardButton::Yes)
//            {
//                closeClassifier();
//            }
//        }

//    });     // live classification
//    connect(classifierWidget, &ClassifierWidget::isLiveChanged, this, &MainWindow::setIsLiveClassificationState);


//    // measurement data changed
//    connect(mData, &MeasurementData::dataChangedSet, this, [this](bool dataChanged){
//        setTitle(dataChanged);
//    });

//    // sensor failures detected
//    connect(absLineGraph, &LineGraphWidget::sensorFailure, this, [this](std::vector<bool> newSensorFailures){
//        Q_ASSERT(newSensorFailures.size() == MVector::nChannels);

//        std::vector<bool> oldSensorFailures = mData->getSensorFailures();
//        std::vector<bool> mergedSensorFailures;

//        for (uint i=0; i<MVector::nChannels; i++)
//            mergedSensorFailures.push_back(oldSensorFailures[i] || newSensorFailures[i]);

//        if (mergedSensorFailures != oldSensorFailures)
//        {
//            mData->setSensorFailures(mergedSensorFailures);
//            measInfoWidget->setFailures(mergedSensorFailures);
//        }
//    }); // absGraph -> mData
//    connect(mData, &MeasurementData::sensorFailuresSet, relLineGraph, &LineGraphWidget::setSensorFailureFlags);    // mData: failures changed -> lGraph: update sensr failures
//    connect(mData, &MeasurementData::sensorFailuresSet, absLineGraph, &LineGraphWidget::setSensorFailureFlags);    // mData: failures changed -> absLGraph: update sensor failures
//    connect(mData, &MeasurementData::sensorFailuresSet, this, [this](std::vector<bool>){
//        auto functionalisation = mData->getFunctionalisation();
//        auto sensorFailures = mData->getSensorFailures();

//        if (mData->getFuncMap(functionalisation, sensorFailures).size() > 1)
//        {
//            // update func line graph:
//            funcLineGraph->clearGraph();

//            auto data = mData->getRelativeData();

//            funcLineGraph->setData(mData->getFuncData(), functionalisation, sensorFailures);

//            // update func bar graphs:
//            if (!mData->getSelectionMap().isEmpty())
//            {
//                funcBarGraph->clearBars();
//                MVector selectionVector = mData->getSelectionVector();

//                funcBarGraph->setBars(selectionVector, sensorFailures, functionalisation);
//            }
//        } else
//            funcLineGraph->setSensorFailureFlags(mData->getSensorFailures());
//    });     // update functionalisation graphs





//    // saving images of the graphs
//    connect(relLineGraph, &LineGraphWidget::ImageSaveRequested, this, [this](){
//        saveLineGraph(relLineGraph);
//    });

//    connect(funcLineGraph, &LineGraphWidget::ImageSaveRequested, this, [this](){
//        saveLineGraph(funcLineGraph);
//    });

//    connect(absLineGraph, &LineGraphWidget::ImageSaveRequested, this, [this](){
//        saveLineGraph(absLineGraph);
//    });

//    connect(vectorBarGraph, &BarGraphWidget::imageSaveRequested, this, [this](){
//        saveBarGraph(vectorBarGraph);
//    });

//    connect(funcBarGraph, &BarGraphWidget::imageSaveRequested, this, [this](){
//        saveBarGraph(funcBarGraph);
//    });

//    // functionalisation changed
//    connect(mData, &MeasurementData::functionalisationChanged, this, &MainWindow::updateFuncGraph); // recalculate func line graph

//    // timer for autosaves
//    connect(&autosaveTimer, &QTimer::timeout, this, &MainWindow::updateAutosave);
//    autosaveTimer.setSingleShot(false);
//    autosaveTimer.start(static_cast<int>(autosaveIntervall * 60 * 1000));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clearGraphs()
{
    funcLineGraph->clearGraph();
    relLineGraph->clearGraph();
    absLineGraph->clearGraph();
}

void MainWindow::addVector(uint timestamp, AbsoluteMVector absoluteVector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    absLineGraph->addVector(timestamp, absoluteVector, functionalisation, sensorFailures);

    RelativeMVector relativeVector = absoluteVector.getRelativeVector();
    relLineGraph->addVector(timestamp, relativeVector, functionalisation, sensorFailures);

    RelativeMVector funcVector = relativeVector.getFuncVector(functionalisation, sensorFailures);
    funcLineGraph->addVector(timestamp, funcVector, functionalisation, sensorFailures);
}

void MainWindow::setData(const QMap<uint, AbsoluteMVector> &data, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    absLineGraph->clearGraph();
    relLineGraph->clearGraph();
    funcLineGraph->clearGraph();

    absLineGraph->setReplotStatus(false);
    relLineGraph->setReplotStatus(false);
    funcLineGraph->setReplotStatus(false);

    for (uint timestamp : data.keys())
    {
        absLineGraph->addVector(timestamp, data[timestamp], functionalisation, sensorFailures);

        RelativeMVector relVector = data[timestamp].getRelativeVector();
        relLineGraph->addVector(timestamp, relVector, functionalisation, sensorFailures);
        funcLineGraph->addVector(timestamp, relVector.getFuncVector(functionalisation, sensorFailures), functionalisation, sensorFailures);
    }

    absLineGraph->setReplotStatus(true);
    relLineGraph->setReplotStatus(true);
    funcLineGraph->setReplotStatus(true);

    absLineGraph->zoomToData();
    relLineGraph->zoomToData();
    funcLineGraph->zoomToData();
}

void MainWindow::setStatus(DataSource::Status newStatus)
{
    switch (newStatus) {
    case DataSource::Status::NOT_CONNECTED:
        ui->actionStart->setIcon(QIcon(":/icons/start"));
        ui->actionStart->setEnabled(false);
        ui->actionStart->setChecked(false);
        ui->actionStop->setEnabled(false);
        ui->actionReset->setEnabled(false);
        ui->actionReconnect->setEnabled(false);
        ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/disconnected"));
        statusTextLabel->setText("Sensor Status: Not Connected");
        statusImageLabel->setPixmap(QPixmap(":/icons/disconnected"));
        break;
    case DataSource::Status::CONNECTING:
        ui->actionStart->setIcon(QIcon(":/icons/start"));
        ui->actionStart->setEnabled(false);
        ui->actionStart->setChecked(false);
        ui->actionStop->setEnabled(false);
        ui->actionReset->setEnabled(false);
        ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/disconnected"));
        ui->actionReconnect->setEnabled(false);
        statusTextLabel->setText("Sensor Status: Connecting...");
        statusImageLabel->setPixmap(QPixmap(":/icons/baseVector"));
        break;
    case DataSource::Status::CONNECTED:
        ui->actionStart->setIcon(QIcon(":/icons/start"));
        ui->actionStart->setEnabled(true);
        ui->actionStart->setChecked(false);
        ui->actionStop->setEnabled(false);
        ui->actionReset->setEnabled(false);
        ui->actionReconnect->setEnabled(false);
        ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/connected"));
        statusTextLabel->setText("Sensor Status: Connected");
        statusImageLabel->setPixmap(QPixmap(":/icons/connected"));
        break;
    case DataSource::Status::SET_BASEVECTOR:
        ui->actionStart->setIcon(QIcon(":/icons/paused"));
        ui->actionStart->setEnabled(false);
        ui->actionStart->setChecked(false);
        ui->actionStop->setEnabled(false);
        ui->actionReset->setEnabled(false);
        ui->actionReconnect->setEnabled(false);
        ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/connected"));
        statusTextLabel->setText("Sensor Status: Setting Base Vector (R0)...");
        statusImageLabel->setPixmap(QPixmap(":/icons/baseVector"));
        break;
    case DataSource::Status::RECEIVING_DATA:
        ui->actionStart->setIcon(QIcon(":/icons/paused"));
        ui->actionStart->setEnabled(true);
        ui->actionStart->setChecked(false);
        ui->actionStop->setEnabled(true);
        ui->actionReset->setEnabled(true);
        ui->actionReconnect->setEnabled(false);
        ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/connected"));
        statusTextLabel->setText("Sensor Status: Receiving Data");
        statusImageLabel->setPixmap(QPixmap(":/icons/recording"));
        break;
    case DataSource::Status::CONNECTION_ERROR:
        ui->actionStart->setEnabled(false);
        ui->actionStart->setChecked(false);
        ui->actionStop->setEnabled(false);
        ui->actionReset->setEnabled(false);
        ui->actionReconnect->setEnabled(true);
        ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/disconnected"));
        statusTextLabel->setText("Sensor Status: Error");
        statusImageLabel->setPixmap(QPixmap(":/icons/error"));
        break;
    case DataSource::Status::PAUSED:
        ui->actionStart->setIcon(QIcon(":/icons/start"));
        ui->actionStart->setEnabled(true);
        ui->actionStart->setChecked(true);
        ui->actionStop->setEnabled(true);
        ui->actionReset->setEnabled(true);
        ui->actionReconnect->setEnabled(false);
        ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/connected"));
        statusTextLabel->setText("Sensor Status: Paused");
        statusImageLabel->setPixmap(QPixmap(":/icons/paused"));
        break;
    default:
        Q_ASSERT("Unknown Sensor Status!" && false);
    }
}

void MainWindow::on_actionSave_Data_As_triggered()
{
    emit saveDataAsRequested();
}

void MainWindow::on_actionsave_selection_triggered()
{
    emit saveSelectionRequested();
}

//void MainWindow::saveLineGraph(LineGraphWidget *graph)
//{
//    // load export directory
//    QString exportDir = settings->value(EXPORT_DIR_KEY, DEFAULT_EXPORT_DIR).toString();

//    // save file dialog
//    QString selectedExtension;
//    QString filename = QFileDialog::getSaveFileName(this, tr("Save Graph Image"), exportDir, "Image Files (*.png *.jpg *.jpeg *.bmp)", &selectedExtension);

//    // save image
//    if (!filename.isEmpty())
//    {
//        // add extension if none was set
//        QStringList splitList = filename.split(".");
//        if (splitList.size() < 2)
//            filename += ".jpg";
//        // unknown file extension
//        else if (splitList.last() != "png" && splitList.last() != "jpg" && splitList.last() != "jpeg" && splitList.last() != "bmp")
//            filename += ".jpg";

//        graph->saveImage(filename);

//        // save export dir
//        QStringList filePathList = filename.split("/");
//        filePathList.removeLast();
//        QString newExportDir = filePathList.join("/");
//        if (newExportDir != exportDir)
//        {
//            settings->setValue(EXPORT_DIR_KEY, newExportDir);
//            settings->sync();
//        }
//    }
//}

//void MainWindow::saveBarGraph(BarGraphWidget *graph)
//{
//    // load export directory
//    QString exportDir = settings->value(EXPORT_DIR_KEY, DEFAULT_EXPORT_DIR).toString();

//    // save file dialog
//    QString selectedExtension;
//    QString filename = QFileDialog::getSaveFileName(this, tr("Save Graph Image"), exportDir, "Image Files (*.png *.jpg *.jpeg *.bmp)", &selectedExtension);

//    // save image
//    if (!filename.isEmpty())
//    {
//        // add extension if none was set
//        QStringList splitList = filename.split(".");
//        if (splitList.size() < 2)
//            filename += ".jpg";
//        // unknown file extension
//        else if (splitList.last() != "png" && splitList.last() != "jpg" && splitList.last() != "jpeg" && splitList.last() != "bmp")
//            filename += ".jpg";

//        graph->saveImage(filename);

//        // save export dir
//        QStringList filePathList = filename.split("/");
//        filePathList.removeLast();
//        QString newExportDir = filePathList.join("/");
//        if (newExportDir != exportDir)
//        {
//            settings->setValue(EXPORT_DIR_KEY, newExportDir);
//            settings->sync();
//        }
//    }
//}

//void MainWindow::resetNChannels(uint newNChannels)
//{
//    // change nChannels
//    MVector::nChannels = newNChannels;

//    // reset mData
//    mData->resetNChannels();

//    // reset graphs
//    // funcLineGraph will be updated anyways
//    relLineGraph->setNChannels(MVector::nChannels);
//    absLineGraph->setNChannels(MVector::nChannels);
//    vectorBarGraph->resetNChannels();
//    funcBarGraph->resetNChannels();
//}

void MainWindow::on_actionLoad_triggered()
{
    emit loadMeasurementRequested();
}

void MainWindow::on_actionSet_USB_Connection_triggered()
{
    emit setConnectionRequested();
}

void MainWindow::on_actionSettings_triggered()
{
    emit generalSettingsRequested();
}

void MainWindow::on_actionStart_triggered()
{
    absLineGraph->setMeasRunning(true);
    relLineGraph->setMeasRunning(true);
    funcLineGraph->setMeasRunning(true);

    emit startRequested();
}

void MainWindow::on_actionStop_triggered()
{
    absLineGraph->setMeasRunning(false);
    relLineGraph->setMeasRunning(false);
    funcLineGraph->setMeasRunning(false);

    emit stopRequested();
}

void MainWindow::on_actionReset_triggered()
{
    emit resetRequested();
}

void MainWindow::on_actionAnnotate_selection_triggered()
{
    emit selectionGroundTruthAnnotationRequested();
}

void MainWindow::on_actionSet_detected_class_of_selection_triggered()
{
    emit selectionDetectionAnnotationRequested();
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    if (dataIsChanged)
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question( this, "eNoseAnnotator",
                                                                    tr("The measurement data was changed without saving.\nDo you want to save the measurement before leaving?\n"),
                                                                   QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
        if (resBtn == QMessageBox::Yes)
        {
            emit saveDataRequested();
            event->accept();
        } else if (resBtn == QMessageBox::Cancel)
        {
            event->ignore();
        } else
        {
            event->accept();
        }
    }
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::on_actionSave_triggered()
{
    emit saveDataRequested();
}

void MainWindow::on_actionAbout_triggered()
{
    std::stringstream ss;
    QString gitCommit(GIT_VERSION); // git has to be on path in order to be set!
    ss << "<a href='https://github.com/Tilagiho/eNoseAnnotator'>eNoseAnnotator</a> Version: " << gitCommit.toStdString() << "<br><br>Compiled with QT Version " << QT_VERSION_STR << "<br>Graphs made using <a href='https://www.qcustomplot.com/'>QCustomPlot</a><br><br>";
    QString appCredits = ss.str().c_str();
    QString iconCredits = "Application Icon made by: Timo Land<br>USB icons from <a href='https://www.icons8.de'>Icons8</a><br>All other icons made by <a href='https://smashicons.com'>SmashIcons</a> from <a href='https://www.flaticon.com'>www.flaticon.com</a>";

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("About eNoseAnnotator");
    msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
    msgBox.setText(appCredits + iconCredits);
    msgBox.exec();
}

void MainWindow::sensorConnected(QString sensorId)
{
    // info widget
    measInfoWidget->setSensorId(sensorId);

    // tool bar
    ui->actionStart->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->actionReset->setEnabled(false);
    ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/disconnected"));

    // status bar
    statusTextLabel->setText("Sensor Status: Connecting...");
    statusImageLabel->setPixmap(QPixmap(":/icons/baseVector"));
}

void MainWindow::redrawFuncGraph(const QMap<uint, AbsoluteMVector> &data, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    // store interval
    QwtInterval axisIntv = funcLineGraph->axisInterval(QwtPlot::xBottom);

    // redraw func graph with updated func vectors
    funcLineGraph->setReplotStatus(false);
    funcLineGraph->clearGraph();
    for (uint timestamp : data.keys())
    {
        MVector funcVector = data[timestamp].getRelativeVector().getFuncVector(functionalisation, sensorFailures);
        funcLineGraph->addVector(timestamp, funcVector, functionalisation, sensorFailures);
    }

    funcLineGraph->setupLegend(functionalisation, sensorFailures);
    funcLineGraph->setReplotStatus(true);

    // restore x axis interval
    funcLineGraph->setAxisIntv(axisIntv, QwtPlot::xBottom);

    // restore selection in func graph
    QPair<double, double> selectionIntv = absLineGraph->getSelectionRange();
    funcLineGraph->makeSelection(selectionIntv.first, selectionIntv.second);
}

void MainWindow::setSelectionActionsEnabled(bool selectionMade)
{
    ui->actionAnnotate_selection->setEnabled(selectionMade);
    ui->actionsave_selection->setEnabled(selectionMade);
    ui->actionDelete_Annotation->setEnabled(selectionMade);
    ui->actionSet_detected_class_of_selection->setEnabled(selectionMade);
    ui->actionFit_curve->setEnabled(selectionMade);
}

bool MainWindow::getConverterRunning() const
{
    return converterRunning;
}

void MainWindow::setFunctionalisation(const QMap<uint, AbsoluteMVector> &data, Functionalisation &functionalisation, std::vector<bool> &sensorFailures)
{
    // info widget
    measInfoWidget->setFunctionalisation(functionalisation);

    // update relative line graph colors
    relLineGraph->setFunctionalisation(functionalisation, sensorFailures);
    absLineGraph->setFunctionalisation(functionalisation, sensorFailures);

    // recalculate funcLineGraph
    redrawFuncGraph(data, functionalisation, sensorFailures);
}

void MainWindow::setSensorFailures(const QMap<uint, AbsoluteMVector> &data, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    // info widget
    measInfoWidget->setSensorFailures(sensorFailures);

    // line graphs
    absLineGraph->setSensorFailures(sensorFailures, functionalisation);
    relLineGraph->setSensorFailures(sensorFailures, functionalisation);

    absLineGraph->replot();
    relLineGraph->replot();

    redrawFuncGraph(data, functionalisation, sensorFailures);
}

void MainWindow::openSensorFailuresDialog(const std::vector<bool> &sensorFailures)
{
    SetSensorFailuresDialog *sfDialog = new SetSensorFailuresDialog(sensorFailures, this);
    sfDialog->setWindowTitle("Sensor failure flags");

    if (sfDialog->exec())
    {
        emit sensorFailuresSet(sfDialog->getSensorFailures());
    }

    sfDialog->deleteLater();
}

void MainWindow::setDataChanged(bool value, QString filename)
{
    // update isChange
    dataIsChanged = value;

    // set window title
//    QString filename = mData->getSaveFilename();

    QString title;
    QString titleExtension;

    if (filename != "")
    {
        QDir dataDir{DEFAULT_DATA_DIR};
        QFileInfo fileInfo(filename);
        QString extension;
        if (fileInfo.absolutePath().startsWith(dataDir.absolutePath()))
            titleExtension = " - " + dataDir.relativeFilePath(filename);
        else
            titleExtension = " - .../" + fileInfo.fileName();
    }

   if (dataIsChanged)
   {
        ui->actionSave->setEnabled(true);
        title = "eNoseAnnotator*";
   } else
   {
       ui->actionSave->setEnabled(false);
       title = "eNoseAnnotator";
   }
   this->setWindowTitle(title + titleExtension);
}

void MainWindow::setClassifier(QString name, QStringList classNames, bool isInputAbsolute, QString presetName)
{
    // update menu
    ui->actionClassify_measurement->setEnabled(true);
    ui->actionCloseClassifier->setEnabled(true);

    // set classifier widget
    classifierWidget->setClassifier(name, classNames, isInputAbsolute, presetName);
}

void MainWindow::closeClassifier()
{
    classifierWidget->clear();
    ui->actionCloseClassifier->setEnabled(false);
    ui->actionClassify_measurement->setEnabled(false);
}

void MainWindow::changeAnnotations( const QMap<uint, Annotation> annotations , bool isUserAnnotation )
{
    absLineGraph->setAnnotations(annotations, isUserAnnotation);
    relLineGraph->setAnnotations(annotations, isUserAnnotation);
    funcLineGraph->setAnnotations(annotations, isUserAnnotation);
}

void MainWindow::setSelectionVector ( const AbsoluteMVector &vector, const std::vector<bool> &sensorFailures, const Functionalisation &functionalisation )
{
    auto relVector = vector.getRelativeVector();
    vectorBarGraph->setVector(relVector, sensorFailures, functionalisation);
    funcBarGraph->setVector(relVector.getFuncVector(functionalisation, sensorFailures), sensorFailures, functionalisation);
}

void MainWindow::clearSelectionVector()
{
    vectorBarGraph->clear();
    funcBarGraph->clear();
}

void MainWindow::setTitle( QString filename, bool dataChanged )
{
    QString title;
    QString titleExtension;

    if (filename != "")
    {
        QDir dataDir{DEFAULT_DATA_DIR};
        QFileInfo fileInfo(filename);
        QString extension;
        if (fileInfo.absolutePath().startsWith(dataDir.absolutePath()))
            titleExtension = " - " + dataDir.relativeFilePath(filename);
        else
            titleExtension = " - .../" + fileInfo.fileName();
    }

   if (dataChanged)
   {
        ui->actionSave->setEnabled(true);
        title = "eNoseAnnotator*";
   } else
   {
       ui->actionSave->setEnabled(false);
       title = "eNoseAnnotator";
   }
   setWindowTitle(title + titleExtension);
}

void MainWindow::createDockWidgets()
{
    setDockNestingEnabled(true);

    // create graph widgets & their docks
    // func line graph
    QDockWidget *flgdock = ui->dock1;
    funcLineGraph = new FuncLineGraphWidget;  // init func line Graph
    flgdock->setAllowedAreas(Qt::LeftDockWidgetArea);
    flgdock->setWidget(funcLineGraph);
    addDockWidget(Qt::LeftDockWidgetArea, flgdock);
    leftDocks << flgdock;

    // relative line graph
    QDockWidget *rlgdock = new QDockWidget(tr("Relative Line Graph"), this);
    relLineGraph = new RelativeLineGraphWidget;
    rlgdock->setAllowedAreas(Qt::LeftDockWidgetArea);
    rlgdock->setWidget(relLineGraph);
    addDockWidget(Qt::LeftDockWidgetArea, rlgdock);
    leftDocks << rlgdock;
//    rlgdock->hide();

    // absolute line graph
    QDockWidget *algdock = new QDockWidget(tr("Absolute Line Graph"), this);
    absLineGraph = new AbsoluteLineGraphWidget;
    algdock->setAllowedAreas(Qt::LeftDockWidgetArea);
    algdock->setWidget(absLineGraph);
    addDockWidget(Qt::LeftDockWidgetArea, algdock);
    leftDocks << algdock;
//    algdock->hide();

    // sync x-range of line graphs
    connect(absLineGraph, &LineGraphWidget::axisIntvSet, relLineGraph, &LineGraphWidget::setAxisIntv);
    connect(absLineGraph, &LineGraphWidget::axisIntvSet, funcLineGraph, &LineGraphWidget::setAxisIntv);
    connect(relLineGraph, &LineGraphWidget::axisIntvSet, absLineGraph, &LineGraphWidget::setAxisIntv);
    connect(relLineGraph, &LineGraphWidget::axisIntvSet, funcLineGraph, &LineGraphWidget::setAxisIntv);
    connect(funcLineGraph, &LineGraphWidget::axisIntvSet, absLineGraph, &LineGraphWidget::setAxisIntv);
    connect(funcLineGraph, &LineGraphWidget::axisIntvSet, relLineGraph, &LineGraphWidget::setAxisIntv);

    // selection flow:
    connect(absLineGraph, &LineGraphWidget::selectionMade, this, &MainWindow::selectionMade);
    connect(absLineGraph, &LineGraphWidget::selectionMade, this, [this](double, double){
        setSelectionActionsEnabled(true);
    });
    connect(absLineGraph, &LineGraphWidget::selectionCleared, this, &MainWindow::selectionCleared);
    connect(absLineGraph, &LineGraphWidget::selectionCleared, this, [this](){
        setSelectionActionsEnabled(false);
    });

    // sync selection between graphs
    connect(absLineGraph, SIGNAL(selectionMade(uint, uint)), relLineGraph, SLOT(makeSelection(uint, uint)));
    connect(absLineGraph, SIGNAL(selectionMade(uint, uint)), absLineGraph, SLOT(makeSelection(uint, uint)));
    connect(relLineGraph, SIGNAL(selectionMade(uint, uint)), absLineGraph, SLOT(makeSelection(uint, uint)));
    connect(relLineGraph, SIGNAL(selectionMade(uint, uint)), funcLineGraph, SLOT(makeSelection(uint, uint)));
    connect(funcLineGraph, SIGNAL(selectionMade(uint, uint)), absLineGraph, SLOT(makeSelection(uint, uint)));
    connect(funcLineGraph, SIGNAL(selectionMade(uint, uint)), relLineGraph, SLOT(makeSelection(uint, uint)));

    connect(absLineGraph, &LineGraphWidget::selectionCleared, relLineGraph, &LineGraphWidget::clearSelection);
    connect(absLineGraph, &LineGraphWidget::selectionCleared, funcLineGraph, &LineGraphWidget::clearSelection);
    connect(relLineGraph, &LineGraphWidget::selectionCleared, absLineGraph, &LineGraphWidget::clearSelection);
    connect(relLineGraph, &LineGraphWidget::selectionCleared, funcLineGraph, &LineGraphWidget::clearSelection);
    connect(funcLineGraph, &LineGraphWidget::selectionCleared, absLineGraph, &LineGraphWidget::clearSelection);
    connect(funcLineGraph, &LineGraphWidget::selectionCleared, relLineGraph, &LineGraphWidget::clearSelection);

//    // vector bar graph
    QDockWidget *vbgdock = ui->dock2;
    vectorBarGraph = new RelVecBarGraphWidget;
    vbgdock->setAllowedAreas(Qt::LeftDockWidgetArea);
    vbgdock->setWidget(vectorBarGraph);
    addDockWidget(Qt::LeftDockWidgetArea, vbgdock);
    leftDocks << vbgdock;
//    vbgdock->hide();

    // functionalisation bar graph
    QDockWidget *fbgdock = new QDockWidget(tr("Functionalisation Bar Graph"), this);
    funcBarGraph = new FuncBarGraphWidget;
    fbgdock->setAllowedAreas(Qt::LeftDockWidgetArea);
    fbgdock->setWidget(funcBarGraph);
    addDockWidget(Qt::LeftDockWidgetArea, fbgdock);
    leftDocks << fbgdock;

    // add actions to view menu
    ui->menuView->addAction(flgdock->toggleViewAction());
    ui->menuView->addAction(fbgdock->toggleViewAction());
    ui->menuView->addAction(rlgdock->toggleViewAction());
    ui->menuView->addAction(algdock->toggleViewAction());
    ui->menuView->addAction(vbgdock->toggleViewAction());

    // create tabs
    tabifyDockWidget(algdock, rlgdock);
    tabifyDockWidget(rlgdock, flgdock);
    tabifyDockWidget(fbgdock, vbgdock);

    // right widgets
    measInfoWidget = static_cast<InfoWidget*>(ui->infoWidget);
    classifierWidget = static_cast<ClassifierWidget*>(ui->classifierInfoWidget);

    connect(measInfoWidget, &InfoWidget::setSensorFailuresRequested, this, &MainWindow::sensorFailureDialogRequested);
    connect(measInfoWidget, &InfoWidget::setFunctionalitionRequested, this, &MainWindow::functionalisationDialogRequested);

    int dockWidth = 0.7 * window()->size().width();
    for (auto dock : leftDocks)
        dock->resize(dockWidth, dock->size().height());
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    auto resizedDock = static_cast<QDockWidget*>(obj);

  if (event->type() == QEvent::Resize && leftDocks.contains(resizedDock))
  {
        auto resizeEvent = static_cast<QResizeEvent*>(event);
        int newWidth = window()->size().width() - resizeEvent->size().width() - 5;
//        measInfoWidget->resize(newWidth, measInfoWidget->size().width());
//        classifierWidget->resize(newWidth, classifierWidget->size().width());
  }
  return QWidget::eventFilter(obj, event);
}

void MainWindow::on_actionLoadClassifier_triggered()
{
    emit loadClassifierRequested();
}

//void MainWindow::updateFuncGraph()
//{
//    auto data = mData->getRelativeData();
//    auto sensorFailures = mData->getSensorFailures();
//    auto functionalisation = mData->getFunctionalisation();

//    // get number of funcs
//    auto funcMap = mData->getFuncMap(functionalisation, sensorFailures);
//    int funcSize = funcMap.size();

//    // no funcs set:
//    // use normal graph
//    if (funcMap.size() == 1)
//    {
//        if (funcLineGraph->getNChannels() != MVector::nChannels)
//        {
//            funcLineGraph->clearGraph();
//            funcLineGraph->setNChannels(MVector::nChannels);
//        }
//        funcLineGraph->setData(data, functionalisation, sensorFailures);
//    }
//    // else: reset graph
//    else
//    {
//        if (funcLineGraph->getNChannels() != funcSize)
//        {
//            // store xAxis range
//            auto oldRange = funcLineGraph->getXRange();

//            // reset funcLineGraph
//            funcLineGraph->clearGraph();
//            funcLineGraph->resetGraph(funcSize);

//            // restore xAxis range
//            funcLineGraph->setXRange(oldRange);
//        }

//        funcLineGraph->setData(mData->getFuncData(), functionalisation, sensorFailures);
//    }

//    // update func bar graph
//    if (!mData->getSelectionMap().isEmpty())
//    {
//        MVector selectionVector = mData->getSelectionVector();

//        funcBarGraph->setBars(selectionVector, sensorFailures, functionalisation);
//    }

//    // reset graph pens
//    relLineGraph->resetColors();
//    absLineGraph->resetColors();
//    vectorBarGraph->resetColors();
//}

/*!
 * \brief MainWindow::connectFLGraph makes connections for funcLineGraph. has to be called each time funcLineGraph is recreated.
 */
//void MainWindow::connectFLGraph()
//{
//    // xRange
//    connect(funcLineGraph, SIGNAL(xRangeChanged(QCPRange)), relLineGraph, SLOT(setXRange(QCPRange)));
//    connect(relLineGraph, SIGNAL(xRangeChanged(QCPRange)), funcLineGraph, SLOT(setXRange(QCPRange)));

//    // selection
//    connect(funcLineGraph, &LineGraphWidget::dataSelectionChanged, absLineGraph, &LineGraphWidget::setSelection);
//    connect(funcLineGraph, &LineGraphWidget::selectionCleared, absLineGraph, &LineGraphWidget::clearSelection);
//    connect(absLineGraph, &LineGraphWidget::dataSelectionChanged, funcLineGraph, &LineGraphWidget::setSelection);
//    connect(absLineGraph, &LineGraphWidget::selectionCleared, funcLineGraph, &LineGraphWidget::clearSelection);
//    connect(relLineGraph, &LineGraphWidget::dataSelectionChanged, funcLineGraph, &LineGraphWidget::setSelection);
//    connect(relLineGraph, &LineGraphWidget::selectionCleared, funcLineGraph, &LineGraphWidget::clearSelection);

//    connect(mData, &MeasurementData::lgClearSelection, funcLineGraph, &LineGraphWidget::clearSelection);

//    // labels
//    connect(mData, &MeasurementData::labelsUpdated, funcLineGraph, &LineGraphWidget::labelSelection); // draw selection and classes

//    // replot status
//    connect(mData, &MeasurementData::setReplotStatus, funcLineGraph, &LineGraphWidget::setReplotStatus);   // replotStatus
//}

void MainWindow::on_actionDelete_Annotation_triggered()
{
    emit deleteGroundTruthAnnotationRequested();
}

void MainWindow::on_actionReconnect_triggered()
{
    emit reconnectRequested();
}

void MainWindow::on_actionClassify_measurement_triggered()
{
    emit classifyMeasurementRequested();
}

void MainWindow::on_actionLive_classifcation_triggered(bool checked)
{
    classifierWidget->setLiveClassification(checked);
}

void MainWindow::setIsLiveClassificationState(bool isLive)
{
    ui->actionLive_classifcation->setChecked(isLive);
    classifierWidget->setLiveClassification(isLive);
}

void MainWindow::on_actionCloseClassifier_triggered()
{
    closeClassifier();
}

void MainWindow::on_actionConverter_triggered()
{
    // block autoSaving while the converter is running
    converterRunning = true;
    ConvertWizard* convertWizard = new ConvertWizard(this);
    convertWizard->exec();
    converterRunning = false;
}

void MainWindow::on_actionFit_curve_triggered()
{
    emit fitCurvesRequested();
}
