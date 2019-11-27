#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "functionalisationdialog.h"
#include "generalsettings.h"
#include "classselector.h"
#include "linegraphwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // DEBUG: set BarGraphWidgetMode
//    ui->bGraph->setMode(BarGraphWidget::Mode::showAll);

    // hide annotation menu points
    // TODO: delete all annotation elements
    ui->actionSaveAnnotation->setVisible(false);
    ui->actionOpenAnnotation->setVisible(false);
    ui->actionFunctionalitization->setVisible(false);

    // hide absolute graph
    ui->absLGraph->hide();

    // prepare menubar
    ui->actionStart->setEnabled(false);
    ui->actionReset->setEnabled(false);
    ui->actionStop->setEnabled(false);

    ui->actionClassify_selection->setEnabled(false);
    ui->actionSet_detected_class_of_selection->setEnabled(false);

    // user can only set detected class in debug mode
    #ifdef QT_NO_DEBUG
    ui->actionSet_detected_class_of_selection->setVisible(false);
    #endif


    mData = new MeasurementData();

    // this->setStyleSheet("QSplitter::handle{background: black;}"); // make splitter visible

    // relative graph: ignore limits (minVal, maxVal)
    ui->lGraph->setUseLimits(false);

    // connections:
    // selection flow
    connect(ui->lGraph, &LineGraphWidget::selectionChanged, mData, &MeasurementData::setSelection); // change selection in mData

    connect(ui->lGraph, &LineGraphWidget::selectionCleared, mData, &MeasurementData::clearSelection); // clear selection in mData
    connect(ui->lGraph, &LineGraphWidget::selectionCleared, ui->bGraph, &BarGraphWidget::clearBars);  // clear vector in bGraph
    connect(mData, &MeasurementData::selectionVectorChanged, ui->bGraph, &BarGraphWidget::setBars);   // plot vector in bGraph
    connect(mData, &MeasurementData::selectionCleared, ui->bGraph, &BarGraphWidget::clearBars); // clear vector in bGraph
    connect(mData, &MeasurementData::lgClearSelection, ui->lGraph, &LineGraphWidget::clearSelection);

    connect(mData, &MeasurementData::labelsUpdated, ui->lGraph, &LineGraphWidget::labelSelection); // draw selection and classes

    connect(mData, &MeasurementData::selectionVectorChanged, this, [this](MVector, std::array<bool, MVector::size>){
        ui->actionClassify_selection->setEnabled(true);
        ui->actionSet_detected_class_of_selection->setEnabled(true);
    }); // show classification actions
    connect(mData, &MeasurementData::selectionCleared, this, [this](){
        ui->actionClassify_selection->setEnabled(false);
        ui->actionSet_detected_class_of_selection->setEnabled(false);
    }); // hide classification actions

    // reset graphs
    connect(mData, &MeasurementData::dataReset, this, [this]{
        ui->lGraph->clearGraph();
        ui->absLGraph->clearGraph();
        ui->bGraph->clearBars();
    }); // clear all graphs when data is reset

    // new data
    connect(mData, &MeasurementData::dataAdded, ui->lGraph, &LineGraphWidget::addMeasurement);    // add new data to lGraph                        // add new data to lGraph
    connect(mData, &MeasurementData::absoluteDataAdded, ui->absLGraph, &LineGraphWidget::addMeasurement); // add new absolute measruement
    connect(mData, &MeasurementData::dataSet, ui->lGraph, &LineGraphWidget::setData);     // set loaded data in lGraph
    connect(mData, &MeasurementData::setReplotStatus, ui->lGraph, &LineGraphWidget::setReplotStatus);   // replotStatus
    connect(mData, &MeasurementData::setReplotStatus, ui->absLGraph, &LineGraphWidget::setReplotStatus);   // replotStatus


    // sensor failures detected
    connect(ui->absLGraph, &LineGraphWidget::sensorFailure, this, [this](int channel){


        auto failures = mData->getSensorFailures();
        if (!failures[channel])
        {
            qDebug() << "sensor failure detected in channel " << QString::number(channel);
            failures[channel] = true;
            mData->setFailures(failures);
            ui->data_info_widget->setFailures(failures);
        }
    }); // absGraph -> mData
    connect(mData, &MeasurementData::sensorFailuresSet, ui->lGraph, &LineGraphWidget::setSensorFailureFlags);    // mData: failures changed -> lGraph: update sensr failures
    connect(mData, &MeasurementData::sensorFailuresSet, ui->absLGraph, &LineGraphWidget::setSensorFailureFlags);    // mData: failures changed -> absLGraph: update sensor failures
    connect(ui->lGraph, &LineGraphWidget::requestRedraw, this, [this]{
        // re-add data to graph
        ui->lGraph->setData(mData->getRelativeData());
    });
    connect(ui->absLGraph, &LineGraphWidget::requestRedraw, this, [this]{
        // re-add data to graph
        ui->absLGraph->setData(mData->getAbsoluteData());
    });
    connect(mData, &MeasurementData::sensorFailuresSet, this, [this]{
        MVector selectionVector = mData->getSelectionVector();

        ui->bGraph->setBars(selectionVector, mData->getSensorFailures(), mData->getFunctionalities());
    }); // reset bars when sensorFailures changed

    // measurement info
    // info -> mData
    connect(ui->data_info_widget, &InfoWidget::mCommentChanged, mData, &MeasurementData::setComment);    // comment changed in infoWidget: change comment in mData
    connect(ui->data_info_widget, SIGNAL(failuresChanged(std::array<bool, 64>)), mData, SLOT(setFailures(std::array<bool, 64>)));   // failures changed in infoWidget: change failures in mData

    // mData -> info
    connect(mData, &MeasurementData::sensorIdSet, ui->data_info_widget, &InfoWidget::setSensor);            // mData: sensorId changed -> InfoWidget: show new sensorId
    connect(mData, &MeasurementData::startTimestempSet, ui->data_info_widget, &InfoWidget::setTimestamp);   // mData: timestamp changed -> InfoWidget: show new timestamp
    connect(mData, &MeasurementData::commentSet, ui->data_info_widget, &InfoWidget::setMComment);            // mData: comment changed -> InfoWidget: show new comment
    connect(mData, &MeasurementData::sensorFailuresSet, ui->data_info_widget, &InfoWidget::setFailures);    // mData: failures changed -> InfoWidget: show new failures

    // set functionalities dialog
    connect(ui->data_info_widget, &InfoWidget::setFunctionalities, this, [this](){
        FunctionalisationDialog dialog;

        dialog.setFunctionalities(mData->getFunctionalities());

        if (dialog.exec())
        {
            mData->setFunctionalities(dialog.getFunctionalities());
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;

    delete mData;
}

void MainWindow::on_actionSave_Data_triggered()
{
    mData->saveData(this);
}

void MainWindow::on_actionsave_selection_triggered()
{
    mData->saveSelection(this);
}

void MainWindow::on_actionLoad_triggered()
{
    mData->loadData(this);
}

void MainWindow::on_lSplitter_splitterMoved(int, int)
{
    ui->rSplitter->setSizes(ui->lSplitter->sizes());
}

void MainWindow::on_rSplitter_splitterMoved(int, int)
{
    ui->lSplitter->setSizes(ui->rSplitter->sizes());
}

void MainWindow::on_actionSet_USB_Connection_triggered()
{
    if (usbSource == nullptr)
    {
        // clear mData
        mData->clear();

        USBSettingsDialog* dialog = new USBSettingsDialog(static_cast<QWidget*>(this->parent()));
        if (dialog->exec())
        {

            usbSource = new USBDataSource(this, dialog->getSettings());

            // connect
            // usb connection
            connect (usbSource, &USBDataSource::newMeasurement, this, [this](QString sensorId){
                mData->clear();
                mData->setSensorId(sensorId);
                ui->data_info_widget->setStatus(USBDataSource::Status::SET_BASELEVEL);
            }); //  usb sensor connected
            connect(usbSource, &USBDataSource::beginSetBaseLevel, this, [this](){
                ui->data_info_widget->setStatus(USBDataSource::Status::SET_BASELEVEL);
            });
            connect(usbSource, &USBDataSource::baseLevelSet, mData, &MeasurementData::setBaseLevel);
            connect(usbSource, &USBDataSource::vectorReceived, this, [this] (uint timestamp, MVector vector) {
                mData->addMeasurement(timestamp, vector);
            } );    // new measurement
            connect(usbSource, &USBDataSource::vectorReceived, this, [this] (uint, MVector) {
                if (ui->data_info_widget->statusSet != USBDataSource::Status::OPEN)
                    ui->data_info_widget->setStatus(USBDataSource::Status::OPEN);
            });
            connect(usbSource, &USBDataSource::serialError, this, [this] () {
                ui->data_info_widget->setStatus(USBDataSource::Status::ERR);
                ui->actionStart->setEnabled(true);
                ui->actionStop->setEnabled(false);
                ui->actionReset->setEnabled(false);
            }); // error
            connect(usbSource, &USBDataSource::serialTimeout, this, [this] () {
                ui->data_info_widget->setStatus(USBDataSource::Status::ERR);
                ui->actionStart->setEnabled(true);
                ui->actionStop->setEnabled(false);
                ui->actionReset->setEnabled(false);
            }); // timeout

            ui->actionStart->setEnabled(true);
        }
    }

    else
        usbSource->changeSettings();
}

void MainWindow::on_actionSettings_triggered()
{
    GeneralSettings dialog;

    // set current settings
    dialog.setMaxVal(ui->absLGraph->getMaxVal());   // max value for absolute values
    dialog.setMinVal(ui->absLGraph->getMinVal());   // min value for absolute values
    dialog.setUseLimits(ui->absLGraph->getUseLimits());
    dialog.setShowAbsGraph(!ui->absLGraph->isHidden());

    dialog.setSaveRawInput(mData->getSaveRawInput());
    dialog.setBarGraphMode(ui->bGraph->getMode());

    if (dialog.exec())
    {
        // get new settings
        // --- save raw output? ---
        mData->setSaveRawInput(dialog.getSaveRawInput());

        // --- limits ---
        // get limits
        int newMaxVal = dialog.getMaxVal();
        int newMinVal = dialog.getMinVal();
        int oldMaxVal = ui->absLGraph->getMaxVal();
        int oldMinVal = ui->absLGraph->getMinVal();

        // get useLimits
        bool newUseLimits = dialog.getUseLimits();
        bool oldUseLimits = ui->absLGraph->getUseLimits();

        // recalc sensor failure flags if limits or useLimits changed
        bool limitsChanged = (newMaxVal != oldMaxVal) || (newMaxVal != oldMinVal);
        bool useLimitsChanged = newUseLimits != oldUseLimits;

        auto sensorFailureFlags = mData->getSensorFailures();

        // 4 cases for change
        // 1. useLimits: true -> false:
        //      set all sensorFailureFlags added by limit violations to false
        // 2. useLimits: false -> true:
        //      find all limit violations and set the according flags to true
        // 3. limits: minVal gets bigger or maxVal smaller
        //      find old violations that are no violations anymore and set flag to false
        // 4. limits: minVal gets smaller or maxVal bigger
        //      find new violations that were no violations and set flag  to true
        if (limitsChanged || useLimitsChanged)
        {
            auto dataMap = mData->getAbsoluteData();

            for (MVector vector : dataMap)
            {
                for (int i = 0; i<MVector::size; i++)
                {
                    // case 1+2
                    if (useLimitsChanged)
                    {
                        if (vector[i] < newMinVal || vector[i] > newMaxVal)
                            sensorFailureFlags[i] = newUseLimits;   // useLimits == true -> set flags, else delete them
                    }
                    // case 3+4
                    else    // limitsChanged
                    {
                        // minVal changed
                        if (newMinVal < oldMinVal)  // case 4
                        {
                            for (int i=0; i<MVector::size; i++)
                                if (vector[i] >= newMinVal && vector[i] < oldMinVal)
                                    sensorFailureFlags[i] = false;
                        } else if (newMinVal > oldMinVal)   // case 3
                        {
                            for (int i=0; i<MVector::size; i++)
                                if (vector[i] < newMinVal && vector[i] >= oldMinVal)
                                    sensorFailureFlags[i] = true;
                        }

                        // maxVal changed
                        if (newMaxVal > oldMaxVal)  // case 4
                        {
                            for (int i=0; i<MVector::size; i++)
                                if (vector[i] <= newMaxVal && vector[i] > oldMaxVal)
                                    sensorFailureFlags[i] = false;
                        } else if (newMaxVal < oldMaxVal)   // case 3
                        {
                            for (int i=0; i<MVector::size; i++)
                                if (vector[i] > newMaxVal && vector[i] <= oldMaxVal)
                                    sensorFailureFlags[i] = true;
                        }
                    }

                }
            }

            // set new values
            ui->absLGraph->setMaxVal(newMaxVal);
            ui->absLGraph->setMinVal(newMinVal);
            mData->setSensorFailures(sensorFailureFlags);
            ui->absLGraph->setUseLimits(newUseLimits);
        }


        // --- show/ hide abs graph ---
        if (dialog.getShowAbsGraph() && ui->absLGraph->isHidden())
            ui->absLGraph->show();
        else if (!dialog.getShowAbsGraph() && !ui->absLGraph->isHidden())
            ui->absLGraph->hide();

        // -- BarGraphWidget::Mode --
        ui->bGraph->setMode(dialog.getBarGraphMode());

    }
}

void MainWindow::on_actionStart_triggered()
{
    Q_ASSERT("Error: No connection was specified!" && usbSource!=nullptr);

    if (usbSource->getSerial()->isOpen())
    {
        usbSource->closeSerialPort();
        qWarning() << "Error: Trying to open connection which is already open!";
     }

    if (mData->changed())
    {
        // save old data
        auto answer = QMessageBox::question(this, "Save measurement data", "Do you want to save the current data before starting a new measurement?");
        if (answer == QMessageBox::StandardButton::Yes)
            mData->saveData(this);
    }
    mData->clear();
    ui->lGraph->clearGraph();
    ui->absLGraph->clearGraph();

    // open serial port
    usbSource->openSerialPort();

    // diasable start action, enable stop & reset action
    ui->actionStart->setEnabled(false);
    ui->actionStop->setEnabled(true);
    ui->actionReset->setEnabled(true);

    qDebug() << "New measurement started!";
}

void MainWindow::on_actionStop_triggered()
{
    if (usbSource == nullptr || !usbSource->getSerial()->isOpen())
    {
        qWarning() << "Error: Trying to close non-open connection";
        return;
    }
    usbSource->closeSerialPort();
    usbSource->reset();

    ui->actionStart->setEnabled(true);
    ui->actionStop->setEnabled(false);
    ui->actionReset->setEnabled(false);

    ui->data_info_widget->setStatus(USBDataSource::Status::NOT_CONNECTED);
}

void MainWindow::on_actionReset_triggered()
{
    if (usbSource == nullptr || !usbSource->getSerial()->isOpen())
    {
        qWarning("Error: Trying to reset non-open connection");
        return;
    }

    usbSource->reset();
}

void MainWindow::on_actionClassify_selection_triggered()
{
    Q_ASSERT(!mData->getSelectionMap().isEmpty());

    ClassSelector* dialog = new ClassSelector(this);
    dialog->setWindowTitle("Select class of selection");
    dialog->setClassList(mData->getClassList());

    // connections
    connect(dialog, &ClassSelector::addClass, mData, &MeasurementData::addClass);
    connect(dialog, &ClassSelector::removeClass, mData, &MeasurementData::removeClass);
    connect(dialog, &ClassSelector::changeClass, mData, &MeasurementData::changeClass);

    if (dialog->exec())
    {
        aClass selectedClass = dialog->getClass();
        Q_ASSERT("Selected class is not part of mData!" && (selectedClass.isEmpty() || mData->getClassList().contains(selectedClass)));

        mData->setUserDefinedClassOfSelection(selectedClass.getName(), selectedClass.getAbreviation());
    }

    // disconnect
    disconnect(dialog, &ClassSelector::addClass, mData, &MeasurementData::addClass);
    disconnect(dialog, &ClassSelector::removeClass, mData, &MeasurementData::removeClass);
    disconnect(dialog, &ClassSelector::changeClass, mData, &MeasurementData::changeClass);
}

void MainWindow::on_actionSet_detected_class_of_selection_triggered()
{
    Q_ASSERT(!mData->getSelectionMap().isEmpty());

    ClassSelector* dialog = new ClassSelector(this);
    dialog->setWindowTitle("[Debug] Select class of detected selection");
    dialog->setClassList(mData->getClassList());

    // connections
    connect(dialog, &ClassSelector::addClass, mData, &MeasurementData::addClass);
    connect(dialog, &ClassSelector::removeClass, mData, &MeasurementData::removeClass);
    connect(dialog, &ClassSelector::changeClass, mData, &MeasurementData::changeClass);

    if (dialog->exec())
    {
        aClass selectedClass = dialog->getClass();
        Q_ASSERT("Selected class is not part of mData!" && (selectedClass.isEmpty() || mData->getClassList().contains(selectedClass)));

        mData->setDetectedClassOfSelection(selectedClass.getName(), selectedClass.getAbreviation());
    }

    // disconnect
    disconnect(dialog, &ClassSelector::addClass, mData, &MeasurementData::addClass);
    disconnect(dialog, &ClassSelector::removeClass, mData, &MeasurementData::removeClass);
    disconnect(dialog, &ClassSelector::changeClass, mData, &MeasurementData::changeClass);
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    if (mData->changed())
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question( this, "eNoseAnnotator",
                                                                    tr("The measurement data was changed without saving.\nDo you want to save the measurement before leaving?\n"),
                                                                   QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
        if (resBtn == QMessageBox::Yes)
        {
            mData->saveData(this);
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
