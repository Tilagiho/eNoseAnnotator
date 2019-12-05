#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "functionalisationdialog.h"
#include "generalsettings.h"
#include "classselector.h"
#include "linegraphwidget.h"
#include "datasource.h"
#include "usbdatasource.h"
#include "sourcedialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/icons/icon"));

    // init statusbar
    statusTextLabel = new QLabel(statusBar());
    statusImageLabel = new QLabel(statusBar());
    statusTextLabel->setText("Sensor status: Not connected ");
    statusImageLabel->setPixmap(QPixmap(":/icons/disconnected"));
    statusImageLabel->setScaledContents(true);
    statusImageLabel->setMaximumSize(16,16);
    statusBar()->addPermanentWidget(statusTextLabel);
    statusBar()->addPermanentWidget(statusImageLabel);

    // hide absolute graph
    ui->absLGraph->hide();
    ui->absLGraph->setIsAbsolute(true);

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
    // connect lGraph & absLGraph
    // xAxis
    connect(ui->lGraph, &LineGraphWidget::xRangeChanged, ui->absLGraph, &LineGraphWidget::setXRange);
    connect(ui->absLGraph, &LineGraphWidget::xRangeChanged, ui->lGraph, &LineGraphWidget::setXRange);
    // selection
    connect(ui->lGraph, &LineGraphWidget::dataSelectionChanged, ui->absLGraph, &LineGraphWidget::setSelection);
    connect(ui->lGraph, &LineGraphWidget::selectionCleared, ui->absLGraph, &LineGraphWidget::clearSelection);
    connect(ui->absLGraph, &LineGraphWidget::dataSelectionChanged, ui->lGraph, &LineGraphWidget::setSelection);
    connect(ui->absLGraph, &LineGraphWidget::selectionCleared, this, [this](){
        // illegal data selection: force selectionCleared signal
        ui->lGraph->setSelection(QCPDataSelection(QCPDataRange(2,1)));
    });

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

    // image saves
    connect(ui->lGraph, &LineGraphWidget::ImageSaveRequested, this, [this](){
        // create export folder
        if(!QDir ("./export").exists())
            QDir().mkdir("./export");

        // save file dialog
        QString selectedExtension;
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Line Graph Image"), "./export", "Image Files (*.png *.jpg *.jpeg *.bmp)", &selectedExtension);

        // save image
        if (filename != "")
        {
            // add extension if none was set
            QStringList splitList = filename.split(".");
            if (splitList.size() < 2)
                filename += ".jpg";
            // unknown file extension
            else if (splitList.last() != "png" && splitList.last() != "jpg" && splitList.last() != "jpeg" && splitList.last() != "bmp")
                filename += ".jpg";

            ui->lGraph->saveImage(filename);
        }
    });

    connect(ui->bGraph, &BarGraphWidget::imageSaveRequested, this, [this](){
        // create export folder
        if(!QDir ("./export").exists())
            QDir().mkdir("./export");

        // save file dialog
        QString selectedExtension;
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Line Graph Image"), "./export", "Image Files (*.png *.jpg *.jpeg *.bmp)", &selectedExtension);

        // save image
        if (filename != "")
        {
            // add extension if none was set
            QStringList splitList = filename.split(".");
            if (splitList.size() < 2)
                filename += ".jpg";
            // unknown file extension
            else if (splitList.last() != "png" && splitList.last() != "jpg" && splitList.last() != "jpeg" && splitList.last() != "bmp")
                filename += ".jpg";

            QMessageBox::StandardButton answer = QMessageBox::question(this, "Save Data", "Do you want to save the selection data that created the bar graph alongside the image?");

            if (answer == QMessageBox::StandardButton::Yes)
            {
                // dataFilemame = filename - image extension + ".csv"
                QStringList list = filename.split(".");
                QString dataFilename = list.mid(0, list.length()-1).join(".") + ".csv";
                mData->saveSelection(this, dataFilename);
            }

            ui->bGraph->saveImage(filename);
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
    SourceDialog* dialog = new SourceDialog(static_cast<QWidget*>(this->parent()));

    // source was set before
    if (source != nullptr)
    {
        // set previous settings
        dialog->setSensorId(mData->getSensorId());

        // usb specific
        if (source->sourceType() == DataSource::SourceType::USB)
        {
            USBDataSource::Settings usbSettings;
            usbSettings.portName = source->identifier();

            dialog->setSourceType(DataSource::SourceType::USB);
            dialog->setUSBSettings(usbSettings);
        }
        else
            Q_ASSERT ("Unknown source selected!" && false);
    }

    if (dialog->exec())
    {
        DataSource::SourceType sourceType = dialog->getSourceType();
        QString identifier = dialog->getIdentifier();
        QString sensorId =dialog->getSensorId();

        // source was set before
        if (source != nullptr)
        {
            // if new source type or identifier:
            if (identifier != source->identifier() || sourceType != source->sourceType())
            {
                // measurement running:
                // -> ask if measurement should be stopped
                if (source->status() == DataSource::Status::RECEIVING_DATA)
                {
                    QMessageBox::StandardButton answer = QMessageBox::question(this, "Stop Measurement", "You selected a new connection while a measurement is running.\nDo you want to stop the measurement in order to use the new connection?");

                    if (answer != QMessageBox::StandardButton::Yes)
                        return; // keep current data source

                    source->stop();
                }
                // delete old source
                delete source;
                source = nullptr;
            }
            // only sensor Id was changed
            else if (sensorId != mData->getSensorId())
            {
                mData->setSensorId(sensorId);
            }
        }

        // source not set before or just deleted:
        // -> init new source
        // usb source:
        if (source == nullptr)
        {
            if (sourceType == DataSource::SourceType::USB)
            {
                USBDataSource::Settings settings;
                settings.portName = identifier;

                source = new USBDataSource(settings);
            }

            // make connections
            makeSourceConnections();

            // update ui
            sensorConnected(sensorId);
        }
    }
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
    Q_ASSERT("Error: No connection was specified!" && source!=nullptr);
    Q_ASSERT("Error: Cannot start non-active connection!" && ((source->status() == DataSource::Status::CONNECTED) || (source->status() == DataSource::Status::CONNECTION_ERROR)));

    // reconnect
    if (source->status() == DataSource::Status::CONNECTION_ERROR)
    {
        qDebug() << "Reconnecting Sensor \"" << source->identifier() << "\"";
        source->reconnect();
    }

    if (mData->changed())
    {
        // save old data
        auto answer = QMessageBox::question(this, "Save measurement data", "Do you want to save the current data before starting a new measurement?");
        if (answer == QMessageBox::StandardButton::Yes)
            mData->saveData(this);
    }

    // clear data, init with sensor id, set dataChanged to false
    QString sensorId = mData->getSensorId();
    clearData();
    mData->setSensorId(sensorId);
    mData->setDataNotChanged();

    source->start();

    qDebug() << "New measurement started!";
}

void MainWindow::on_actionStop_triggered()
{
    Q_ASSERT("Error: No connection was specified!" && source!=nullptr);
    Q_ASSERT("Error: Cannot stop non-active connection!" && source->status() == DataSource::Status::RECEIVING_DATA);
    if (source->sourceType() == DataSource::SourceType::USB)
        Q_ASSERT("Error: Connection is not open!" && static_cast<USBDataSource*>(source)->getSerial()->isOpen());

    source->stop();
}

void MainWindow::on_actionReset_triggered()
{
    Q_ASSERT("Error: No connection was specified!" && source!=nullptr);
    Q_ASSERT("Error: Cannot stop non-active connection!" && source->status() == DataSource::Status::RECEIVING_DATA);
    if (source->sourceType() == DataSource::SourceType::USB)
        Q_ASSERT("Error: Connection is not open!" && static_cast<USBDataSource*>(source)->getSerial()->isOpen());

    source->reset();
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

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::on_actionSave_triggered()
{
    QString filename = mData->getSaveFilename();
    Q_ASSERT("No saveFilename is set in MeasurementData!" && filename != "");

    mData->saveData(this, filename);
}

void MainWindow::on_actionAbout_triggered()
{
    QString iconCredits = "Application Icon made by: Timo Land<br>USB icons from <a href='https://www.icons8.de'>Icons8</a><br>All other icons made by <a href='https://smashicons.com'>SmashIcons</a> from <a href='https://www.flaticon.com'>www.flaticon.com</a>";

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("About eNoseAnnotator");
    msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
    msgBox.setText(iconCredits);
    msgBox.exec();
}

void MainWindow::clearData()
{
    mData->clear();
    ui->lGraph->clearGraph();
    ui->absLGraph->clearGraph();
}

void MainWindow::sensorConnected(QString sensorId)
{
    // mData
    mData->setSensorId(sensorId);

    // info widget
    ui->data_info_widget->setSensor(sensorId);
    ui->data_info_widget->setStatus(DataSource::Status::CONNECTING);

    // tool bar
    ui->actionStart->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->actionReset->setEnabled(false);
    ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/disconnected"));

    // status bar
    statusTextLabel->setText("Sensor Status: Connecting...");
    statusImageLabel->setPixmap(QPixmap(":/icons/baseVector"));
}

void MainWindow::makeSourceConnections()
{
    connect(source, &DataSource::baseVectorSet, mData, &MeasurementData::setBaseLevel);
    connect(source, &DataSource::vectorReceived, mData, [this] (uint timestamp, MVector vector) {
        mData->addMeasurement(timestamp, vector);

        if (ui->data_info_widget->statusSet != DataSource::Status::RECEIVING_DATA)
        {
            statusTextLabel->setText("Sensor Status: Receiving Data");
            statusImageLabel->setPixmap(QPixmap(":/icons/recording"));
        }
    } );    // new measurement

    connect(source, &DataSource::error, this, [this] (QString errorString) {
        QMessageBox::critical(this, "Connection Error", errorString);
    }); // error

    connect(source, &DataSource::statusSet, ui->data_info_widget, &InfoWidget::setStatus);
    connect(source, &DataSource::statusSet, this, [this](DataSource::Status status){
        switch (status) {
            case DataSource::Status::NOT_CONNECTED:
                ui->actionStart->setEnabled(false);
                ui->actionStop->setEnabled(false);
                ui->actionReset->setEnabled(false);
                ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/disconnected"));
                statusTextLabel->setText("Sensor Status: Not Connected");
                statusImageLabel->setPixmap(QPixmap(":/icons/disconnected"));
                break;
            case DataSource::Status::CONNECTING:
                ui->actionStart->setEnabled(false);
                ui->actionStop->setEnabled(false);
                ui->actionReset->setEnabled(false);
                ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/disconnected"));
                statusTextLabel->setText("Sensor Status: Connecting...");
                statusImageLabel->setPixmap(QPixmap(":/icons/baseVector"));
                break;
            case DataSource::Status::CONNECTED:
                ui->actionStart->setEnabled(true);
                ui->actionStop->setEnabled(false);
                ui->actionReset->setEnabled(false);
                ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/connected"));
                statusTextLabel->setText("Sensor Status: Connected");
                statusImageLabel->setPixmap(QPixmap(":/icons/connected"));
                break;
            case DataSource::Status::SET_BASEVECTOR:
                ui->actionStart->setEnabled(false);
                ui->actionStop->setEnabled(true);
                ui->actionReset->setEnabled(false);
                ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/connected"));
                statusTextLabel->setText("Sensor Status: Setting Base Vector (R0)...");
                statusImageLabel->setPixmap(QPixmap(":/icons/baseVector"));
                break;
            case DataSource::Status::RECEIVING_DATA:
                ui->actionStart->setEnabled(false);
                ui->actionStop->setEnabled(true);
                ui->actionReset->setEnabled(true);
                ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/connected"));
                statusTextLabel->setText("Sensor Status: Receiving Data");
                statusImageLabel->setPixmap(QPixmap(":/icons/recording"));
                break;
            case DataSource::Status::CONNECTION_ERROR:
                ui->actionStart->setEnabled(true);
                ui->actionStop->setEnabled(false);
                ui->actionReset->setEnabled(false);
                ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/disconnected"));
                statusTextLabel->setText("Sensor Status: Error");
                statusImageLabel->setPixmap(QPixmap(":/icons/error"));
                break;
            default:
                Q_ASSERT("Unknown Sensor Status!" && false);
        }
    });
}
