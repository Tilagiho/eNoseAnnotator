#include "controler.h"

#include <QtCore>
#include <QMessageBox>
#include <QFileDialog>
#include <QAbstractButton>

#include "../widgets/functionalisationdialog.h"
#include "../widgets/sourcedialog.h"
#include "../widgets/classselector.h"
#include "../widgets/generalsettings.h"
#include "../widgets/setsensorfailuresdialog.h"
#include "../widgets/curvefitwizard.h"

#include "defaultSettings.h"
#include "datasource.h"
#include "usbdatasource.h"
#include "fakedatasource.h"
#include "mvector.h"
#include "enosecolor.h"

Controler::Controler(QObject *parent) :
    QObject(parent),
    w(new MainWindow),
    mData(new MeasurementData(this))
{
    // declare meta types
    qRegisterMetaType<AbsoluteMVector>("AbsoluteMVector");

    // setup mainwindow

    // init mData
    mData->setInputFunctionType(inputFunctionType);

    // eNoseColor singleton:
    // keep sensorFailures and functionalisation updated
    ENoseColor::instance().setFunctionalisation(mData->getFunctionalisation());
    ENoseColor::instance().setSensorFailures(mData->getSensorFailures());

    connect(mData, &MeasurementData::functionalisationChanged, this, [this](){
        auto functionalisation = mData->getFunctionalisation();
        ENoseColor::instance().setFunctionalisation(functionalisation);
    });
    connect(mData, &MeasurementData::sensorFailuresSet, this, [this](const QMap<uint, AbsoluteMVector> &data, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures){
        ENoseColor::instance().setSensorFailures(sensorFailures);
    });

    // init settings
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    // init autosave dir
    QDir temp = QDir::temp();
    QFileInfo tempInfo(temp.absolutePath());

    if (temp.exists() && tempInfo.isDir() && dirIsWriteable(temp))
        autosavePath = QDir::tempPath() + "/" + QCoreApplication::applicationName();
    else
        autosavePath = "./.settings";

    if (!QDir(autosavePath).exists())
        QDir().mkdir(autosavePath);

    //                      //
    // make connections     //
    //                      //

    // graph connections:
    // data
    connect(mData, &MeasurementData::vectorAdded, w, &MainWindow::addVector);    // add new vector to graphs
    connect(mData, &MeasurementData::dataSet, w, &MainWindow::setData);

    // failures
    connect(mData, &MeasurementData::sensorFailuresSet, w, &MainWindow::setSensorFailures);
    connect(w, SIGNAL(sensorFailuresSet(const std::vector<bool> &)), mData, SLOT(setSensorFailures(const std::vector<bool> &)));

    // functionalisation
    connect(w, &MainWindow::functionalisationSet, mData, &MeasurementData::setFunctionalisation);
    connect(mData, &MeasurementData::functionalisationChanged, this, [this](){
        auto data = mData->getAbsoluteData();
        auto functionalisation =  mData->getFunctionalisation();
        auto sensorFailures = mData->getSensorFailures();
        w->setFunctionalisation(data, functionalisation, sensorFailures);
    });

    // annotations
    connect(mData, &MeasurementData::annotationsChanged, w, &MainWindow::changeAnnotations);


    // selection flow
    connect(w, &MainWindow::selectionMade, mData, &MeasurementData::setSelection);
    connect(w, &MainWindow::selectionCleared, mData, &MeasurementData::clearSelection);

    connect(mData, &MeasurementData::selectionVectorChanged, w, &MainWindow::setSelectionVector);
    connect(mData, &MeasurementData::selectionCleared, w, &MainWindow::clearSelectionVector);


    // info widget connections:
    connect(w, &MainWindow::sensorFailureDialogRequested, [this](){
        auto sensorFailures = mData->getSensorFailures();
        w->openSensorFailuresDialog(sensorFailures);
    });
    connect(w, &MainWindow::functionalisationDialogRequested, this, &Controler::selectFunctionalisation);

    connect (mData, &MeasurementData::startTimestempSet, w, &MainWindow::startTimestempSet);
    connect (mData, &MeasurementData::commentSet, w, &MainWindow::commentSet);
    connect (mData, &MeasurementData::sensorIdSet, w, &MainWindow::sensorIdSet);

    // menu bar
    connect(w, &MainWindow::setConnectionRequested, this, &Controler::setSourceConnection);
    connect(w, &MainWindow::startRequested, this, &Controler::startMeasurement);
    connect(w, &MainWindow::stopRequested, this, &Controler::stopMeasurement);
    connect(w, &MainWindow::pauseRequested, this, &Controler::pauseMeasurement);
    connect(w, &MainWindow::resetRequested, this, &Controler::resetMeasurement);
    connect(w, &MainWindow::reconnectRequested, this, &Controler::reconnectMeasurement);

    connect(w, &MainWindow::selectionGroundTruthAnnotationRequested, this, &Controler::annotateGroundTruthOfSelection);
    connect(w, &MainWindow::selectionDetectionAnnotationRequested, this, &Controler::annotateDetectionOfSelection);
    connect(w, &MainWindow::deleteGroundTruthAnnotationRequested, this, &Controler::deleteGroundTruthOfSelection);

    connect(w, &MainWindow::loadMeasurementRequested, this, [this](){
        loadData();
    });
    connect(w, &MainWindow::saveDataRequested, this, [this](){
        saveData(false);
    });
    connect(w, &MainWindow::saveDataAsRequested, this, [this](){
        saveData(true);
    });
    connect(w, &MainWindow::saveSelectionRequested, this, &Controler::saveSelection);

    connect(w, &MainWindow::generalSettingsRequested, this, &Controler::setGeneralSettings);

    connect(w, &MainWindow::loadClassifierRequested, this, &Controler::loadClassifier);
    connect(w, &MainWindow::classifyMeasurementRequested, this, &Controler::classifyMeasurement);

    connect(w, &MainWindow::fitCurvesRequested, this, &Controler::fitCurves);

    // window state
    connect(mData, &MeasurementData::dataChangedSet, this, &Controler::setDataChanged);

    // timer for autosaves
    connect(&autosaveTimer, &QTimer::timeout, this, &Controler::updateAutosave);
    autosaveTimer.setSingleShot(false);
    autosaveTimer.start(static_cast<int>(autosaveIntervall * 60 * 1000));

}

Controler::~Controler()
{
    w->deleteLater();

    mData->deleteLater();

    if (source != nullptr)
        source->deleteLater();
    if (sourceThread != nullptr)
        sourceThread->deleteLater();
    if (classifier != nullptr)
        classifier->deleteLater();
}

MainWindow *Controler::getWindow() const
{
    return w;
}

/*!
 * \brief MainWindow::dirIsWriteable checks if files can be created in the directory by trying to create a dummy file.
 * Necessary, because built-in Qt checks are inconsistent on Windows.
 * \param dir
 * \return
 */
bool Controler::dirIsWriteable(QDir dir)
{
    if (!dir.exists())
        return false;

    // check if dummy file can be created
    QFile file (dir.absolutePath() + "/dummy");
    bool fileCreated = file.open(QIODevice::WriteOnly);

    // clean up
    file.remove();

    return fileCreated;
}

void Controler::initialize()
{
    loadCLArguments();
    loadAutosave();
}

void Controler::loadCLArguments()
{
    // parse launch arguments & load file in first arg (if it exists)
    QStringList arguments = QCoreApplication::arguments();
    qDebug() << "Launched with args: " << arguments;

    if (arguments.size() > 1)
        loadData(arguments[1]);

    if (arguments.size() > 2)
        QMessageBox::warning(w, "Launch error", "The application can only be launched with one argument!");
}

void Controler::loadAutosave()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    // check for autosave
    QFile autosaveFile(autosavePath + "/" + autosaveName);
    if (autosaveFile.exists())
    {
        // create message box
        QMessageBox mBox(w);
        QString infoText = "The program was terminated without saving changes in the measurement.\nDo you want to restore the autosave?";
        mBox.setText(infoText);
        mBox.setWindowTitle("Restore autosave");
        mBox.setIcon(QMessageBox::Question);

        // add buttons
        mBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort);
        mBox.button(QMessageBox::Yes)->setText("Restore");
        mBox.button(QMessageBox::No)->setText("Discard");
        mBox.button(QMessageBox::Abort)->setText("Cancel");

        // add additional file info
        QFileInfo info(autosaveFile);
        QLocale locale = w->locale();
        QString sizeText = locale.formattedDataSize(info.size());
        mBox.setDetailedText("Autosave: " + info.lastModified().toString() + " - " + sizeText);

        // execute mBox & retrieve answer
        auto ans = mBox.exec();
        if (ans == QMessageBox::StandardButton::Yes)
        {
            // dataDir is changed to temp dir when loading autosave
            // -> remember dataDir
            QString dataDir = settings.value(DATA_DIR_KEY, DEFAULT_DATA_DIR).toString();

            loadData(autosaveFile.fileName());
            deleteAutosave();

            // override changed flag, so the autosave can be saved
            mData->setDataChanged(true);

            // restore dataDir
            mData->setSaveFilename(dataDir);
        } else if (ans == QMessageBox::StandardButton::No)
        {
            deleteAutosave();
        }
    }
}

void Controler::loadSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    #ifdef QT_DEBUG
    if (CLEAR_SETTINGS)
        settings.clear();
    #endif

    //    qDebug() << "Settings keys before loading:\n" << settings->allKeys().join("; ");

    if (!settings.contains("settingsInitialised"))
        initSettings();

    // load directory paths
    QString dataDir = settings.value(DATA_DIR_KEY, DEFAULT_DATA_DIR).value<QString>();
    mData->setSaveFilename(dataDir);

    // load general settings
    bool useLimits = settings.value(USE_LIMITS_KEY, DEFAULT_USE_LIMITS).toBool();
    int lowerLimit = settings.value(LOWER_LIMIT_KEY, DEFAULT_LOWER_LIMIT).toInt();
    int upperLimit = settings.value(UPPER_LIMIT_KEY, DEFAULT_UPPER_LIMIT).toInt();

    mData->setLimits(lowerLimit, upperLimit, useLimits);
}

void Controler::initSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    setGeneralSettings();

    settings.setValue("settingsInitialised", true);
}

void Controler::setGeneralSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    GeneralSettingsDialog dialog;

    // load current settings
    bool useLimits = mData->getUseLimits();
    double lowerLimit = mData->getLowerLimit();
    double upperLimit = mData->getUpperLimit();
    QString presetDir = settings.value(PRESET_DIR_KEY, DEFAULT_PRESET_DIR).value<QString>();

    // set current settings
    dialog.setMaxVal(upperLimit);   // max value for absolute values
    dialog.setMinVal(lowerLimit);   // min value for absolute values
    dialog.setUseLimits(useLimits);
    dialog.setPresetDir(presetDir);

    if (dialog.exec())
    {
        // get new settings

        // --- limits ---
        double newUpperLimit = dialog.getMaxVal();
        double newLowerLimit = dialog.getMinVal();
        bool newUseLimits = dialog.getUseLimits();

        mData->setLimits(newLowerLimit, newUpperLimit, newUseLimits);

        // get preset dir
        QString newPresetDir = dialog.getPresetDir();


        // save settings
        settings.setValue(PRESET_DIR_KEY, newPresetDir);

        // copy presets into new preset dir
        if (!settings.contains(PRESET_DIR_KEY) || newPresetDir != presetDir)
        {
            if (!QDir(newPresetDir).exists())
                QDir().mkdir(newPresetDir);

            QString presetSourcePath = QCoreApplication::applicationDirPath() + "/" + PRESET_SOURCE;
            QDirIterator it(presetSourcePath);

            while(it.hasNext())
            {
                QString filePath = it.next();
                QString fileName = filePath.split("/").last();
                QFile::copy(filePath, newPresetDir + "/" + fileName);
            }
        }

        // qDebug() << "Keys after general settings dialog:\n"  << settings.allKeys().join("; ");
    }
}

void Controler::saveData()
{
    saveData(false);
}

void Controler::saveData(bool forceDialog)
{
    QString path = mData->getSaveFilename();

    QString fileName;
    if (path.endsWith(".csv") && !forceDialog)
        fileName = path;
    else
        fileName = QFileDialog::getSaveFileName(w, QString("Save data"), path, "Data files (*.csv)");

    // no file selected
    if (fileName.isEmpty() || fileName.endsWith("/"))
        return;

    if (fileName.split(".").last() != "csv")
        fileName += ".csv";

    try {
        mData->saveData(fileName);
        // update saveFilename
        mData->setSaveFilename(fileName);

        // save dataDir
        saveDataDir();
    } catch (std::runtime_error e) {
        QMessageBox::critical(w, "Error saving measurement", e.what());
    }
}

void Controler::loadData()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    // measurement is running: stop measurement?
    if (source != nullptr && source->measIsRunning())
    {
        if (QMessageBox::question(w, tr("Stop measurement"),
            "A measurement is currently running. Do you want to stop it?") == QMessageBox::Yes)
            stopMeasurement();
        else
            return;
    }
    // ask to save old data
    if (!mData->getAbsoluteData().isEmpty() && mData->isChanged())
    {
        if (QMessageBox::question(w, tr("Save data"),
            "Do you want to save the current measurement before loading data?\t") == QMessageBox::Yes)
            saveData();
    }

    // make data directory
    QString dataDir = settings.value(DATA_DIR_KEY, DEFAULT_DATA_DIR).toString();
    if (!QDir(dataDir).exists())
        QDir().mkdir(dataDir);

    // load data
    QString fileName = QFileDialog::getOpenFileName(w, "Open data file", dataDir, "Data files (*.csv)");

    if (fileName.isEmpty())
        return;    

    loadData(fileName);
    saveDataDir();
}

void Controler::loadData(QString fileName)
{
    FileReader* specificReader = nullptr;
    try {
        // use general reader to get specific reader for the format of filename
        FileReader generalFileReader(fileName, this);
        specificReader = generalFileReader.getSpecificReader();

//        connect(specificReader, &FileReader::resetNChannels, w, &MainWindow::resetNChannels);
        specificReader->readFile();
    }
    catch (std::runtime_error e)
    {
        QMessageBox::critical(w, "Error loading measurement", e.what());
    }

    MeasurementData* newData = specificReader->getMeasurementData();

    int nFuncs = newData->getFunctionalisation().getNFuncs();
    nFuncs = nFuncs == 1 ? MVector::nChannels : nFuncs;

    mData->copyFrom(newData);

    // update title of MainWindow
//    w->setDataChanged(false, mData->getSaveFilename());

    // check compability to loaded classifier
    if (classifier != nullptr)
    {
        auto functionalisation = mData->getFunctionalisation();
        auto sensorFailures = mData->getSensorFailures();
        auto funcMap = functionalisation.getFuncMap(sensorFailures);

        // check func preset
        if (classifier->getN() != funcMap.size() || classifier->getPresetName() != mData->getFunctionalisation().getName())
        {
            QString error_message = "Functionalisation of the data loaded seems to be incompatible with the loaded classifier.\nWas the functionalisation set correctly? Is the classifier compatible with the sensor used?";
            QMessageBox::warning(w, "Classifier error", error_message);
        }
    }

    if (specificReader != nullptr)
        delete specificReader;

    // if saving successfull:
    // delete autosave
    if (!mData->isChanged())
        deleteAutosave();
}

void Controler::setDataChanged(bool value)
{
    w->setDataChanged(value, mData->getSaveFilename());
}

void Controler::saveSelection()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    QString filepath = mData->getSaveFilename();

    QString defaultPath = settings.value(DATA_DIR_KEY, DEFAULT_DATA_DIR).toString();
    QString path = (filepath != defaultPath) ? filepath : defaultPath;
    QString fileName = QFileDialog::getSaveFileName(w, QString("Save selected data"), path, "Data files (*.csv)");

    // no file selected
    if (fileName.isEmpty() || fileName.endsWith("/"))
        return;

    if (fileName.split(".").last() != "csv")
        fileName += ".csv";

    try {
        mData->saveSelection(fileName);

        // save export dir
        QStringList pathList = path.split("/");
        settings.setValue(EXPORT_DIR_KEY, pathList.mid(0, pathList.size()-1).join("/"));
    } catch (std::runtime_error e) {
        QMessageBox::critical(w, "Error saving selection", e.what());
    }
}

void Controler::deleteAutosave()
{
    QFile file (autosavePath + "/" + autosaveName);
    if (file.exists())
        file.remove();
}

void Controler::clearData()
{
    mData->clear();
    w->clearGraphs();
}

void Controler::saveDataDir()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    QString saveFilename =  mData->getSaveFilename();

    settings.setValue(DATA_DIR_KEY, saveFilename);
}

void Controler::selectFunctionalisation()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    FunctionalisationDialog dialog(settings.value(PRESET_DIR_KEY, DEFAULT_PRESET_DIR).toString(), MVector::nChannels, w);
    dialog.presetName = mData->getFunctionalisation().getName();

    dialog.setFunctionalisation(mData->getFunctionalisation());

    if (dialog.exec())
    {
        mData->setFunctionalisation(dialog.getFunctionalisation());
        mData->setFuncName(dialog.presetName);
    }
}


void Controler::setSourceConnection()
{
    SourceDialog* dialog = new SourceDialog(static_cast<QWidget*>(this->parent()));

    // source was set before
    if (source != nullptr)
    {
        // set previous settings
        if (!mData->getSensorId().isEmpty())
            dialog->setSensorId(mData->getSensorId());

        dialog->setNChannels(MVector::nChannels);
        dialog->setTimeout(source->getTimeout());

        // usb specific
        if (source->sourceType() == DataSource::SourceType::USB)
        {
            USBDataSource::Settings usbSettings;
            usbSettings.portName = source->identifier();

            dialog->setSourceType(DataSource::SourceType::USB);
            dialog->setUSBSettings(usbSettings);
        }
        else if (source->sourceType() == DataSource::SourceType::FAKE)
        {
            dialog->setSourceType(DataSource::SourceType::FAKE);
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
            // if new source type, identifier, new nChannels or timeout :
            if (identifier != source->identifier() || sourceType != source->sourceType() || dialog->getNChannels() != MVector::nChannels)
            {
                // measurement running:
               // -> ask if measurement should be stopped
               if (source->measIsRunning())
               {
                   QMessageBox::StandardButton answer = QMessageBox::question(w, "Stop Measurement", "You selected a new connection while a measurement is running.\nDo you want to stop the measurement in order to use the new connection?");

                   if (answer != QMessageBox::StandardButton::Yes)
                       return; // keep current data source

                   QMetaObject::invokeMethod(source, "stop", Qt::QueuedConnection);

                }
                // delete old source
                source->deleteLater();
                source = nullptr;
                sourceThread = nullptr;
            }

            // sensor Id was changed
            else if (sensorId != mData->getSensorId() || dialog->getTimeout() != source->getTimeout())
            {
                mData->setSensorId(sensorId);
                source->setTimeout(dialog->getTimeout());
            }
        }

        // source not set before or just deleted:
        // -> init new source
        if (source == nullptr)
        {
            sourceThread = new QThread();
            // usb source:
            if (sourceType == DataSource::SourceType::USB)
            {
                USBDataSource::Settings usbSettings;
                usbSettings.portName = identifier;

                source = new USBDataSource(usbSettings, dialog->getTimeout(), dialog->getNChannels());
            }
            // fake source:
            else if (sourceType == DataSource::SourceType::FAKE)
            {
                source = new FakeDatasource(dialog->getTimeout(), dialog->getNChannels());
            }

            // run source in separate thread
            source->moveToThread(sourceThread);

            connect(sourceThread, SIGNAL(started()), source, SLOT(started())); // init source when thread was started
            connect(source, SIGNAL(destroyed()), sourceThread, SLOT(quit()));   // end thread when source is deleted
            connect(sourceThread, SIGNAL(finished()), sourceThread, SLOT(deleteLater())); // delete thread when finishes

            sourceThread->start();

            // make connections
            makeSourceConnections();

            // update sensor information
            mData->setSensorId(sensorId);
            w->sensorConnected(sensorId);
        } else if (source->status() == DataSource::Status::CONNECTION_ERROR)
        {
            QMetaObject::invokeMethod(source, "reconnect", Qt::QueuedConnection);
        }
    }

    dialog->deleteLater();
}

void Controler::makeSourceConnections()
{
    connect(source, &DataSource::baseVectorSet, mData, &MeasurementData::setBaseVector);
    connect(source, SIGNAL(vectorReceived(uint, AbsoluteMVector)), mData, SLOT(addVector(uint, AbsoluteMVector)));

//        if (measInfoWidget->statusSet != DataSource::Status::RECEIVING_DATA)
//        {
//            statusTextLabel->setText("Sensor Status: Receiving Data");
//            statusImageLabel->setPixmap(QPixmap(":/icons/recording"));
//        }

    connect(source, &DataSource::error, this, [this] (QString errorString) {
        QMessageBox::critical(w, "Connection Error", errorString);
    }); // error

    connect(source, &DataSource::statusSet, w, &MainWindow::setStatus);
}

void Controler::startMeasurement()
{
    Q_ASSERT("Error: No connection was specified!" && source!=nullptr);

    switch(source->status())
    {
    // measurement running
    // -> pause connection
    case DataSource::Status::RECEIVING_DATA:
    case DataSource::Status::SET_BASEVECTOR:
        QMetaObject::invokeMethod(source, "pause", Qt::QueuedConnection);
        break;

    // measurement paused
    // -> resume measurement
    case DataSource::Status::PAUSED:
        QMetaObject::invokeMethod(source, "start", Qt::QueuedConnection);
        break;

    // no measurement running
    // -> start new measurement
    case DataSource::Status::CONNECTED:
    {
        // save old data if changed
        if (mData->isChanged())
        {
            auto answer = QMessageBox::question(w, "Save measurement data", "Do you want to save the current data before starting a new measurement?");
            if (answer == QMessageBox::StandardButton::Yes)
                saveData();
        }

        // clear data for new measurement
//        clearData();


//        // if nChannels has to be changed
//        if (source->getNChannels() != MVector::nChannels)
//        {
//            resetNChannels(source->getNChannels());
//        }
        // no func set: set functionalisation?
        auto functionalisation = mData->getFunctionalisation();
        auto sensorFailures = mData->getSensorFailures();
        auto funcMap = functionalisation.getFuncMap(sensorFailures);
        if (funcMap.size() == 1)
        {
            auto answer = QMessageBox::question(w, "Set Functionalisation", "The sensor functionalisation was not set.\nDo you want to set it before starting a new measurement?");

            if (answer == QMessageBox::StandardButton::Yes)
                selectFunctionalisation();
        }

        // update funcLineGraph
//        w->updateFuncGraph();

        // clear data, init with sensor id, comment & sensor failures, set dataChanged to false
        QString sensorId = mData->getSensorId();
        QString comment = mData->getComment();

        clearData();

        mData->setSensorId(sensorId);
        mData->setComment(comment);
        mData->setSensorFailures(sensorFailures);
        mData->setDataChanged(false);

        QMetaObject::invokeMethod(source, "start", Qt::QueuedConnection);
//        qDebug() << "New measurement started!";
        break;
    }
    default:
        Q_ASSERT("Error: Cannot start or pause non-active  connection!" && source->status() != DataSource::Status::CONNECTING && source->status() != DataSource::Status::NOT_CONNECTED && source->status() != DataSource::Status::CONNECTION_ERROR);
    }
}

void Controler::stopMeasurement()
{
    Q_ASSERT("Error: No connection was specified!" && source!=nullptr);
    Q_ASSERT("Error: Cannot stop non-active connection!" && source->measIsRunning());
    if (source->sourceType() == DataSource::SourceType::USB)
        Q_ASSERT("Error: Connection is not open!" && static_cast<USBDataSource*>(source)->getSerial()->isOpen());

    QMetaObject::invokeMethod(source, "stop", Qt::QueuedConnection);
}

void Controler::pauseMeasurement()
{
    Q_ASSERT("Error: No connection was specified!" && source!=nullptr);
    Q_ASSERT("Error: Cannot stop non-active connection!" && source->measIsRunning());
    if (source->sourceType() == DataSource::SourceType::USB)
        Q_ASSERT("Error: Connection is not open!" && static_cast<USBDataSource*>(source)->getSerial()->isOpen());

    QMetaObject::invokeMethod(source, "pause", Qt::QueuedConnection);
}

void Controler::resetMeasurement()
{
    Q_ASSERT("Error: No connection was specified!" && source!=nullptr);
    Q_ASSERT("Error: Cannot stop non-active connection!" && source->measIsRunning());
    if (source->sourceType() == DataSource::SourceType::USB)
        Q_ASSERT("Error: Connection is not open!" && static_cast<USBDataSource*>(source)->getSerial()->isOpen());

    QMetaObject::invokeMethod(source, "reset", Qt::QueuedConnection);
}

void Controler::reconnectMeasurement()
{
    Q_ASSERT(source->status() == DataSource::Status::CONNECTION_ERROR);
    // reconnect sensor
//    qDebug() << "Reconnecting Sensor \"" << source->identifier() << "\"";

    QMetaObject::invokeMethod(source, "reconnect", Qt::QueuedConnection);
}

void Controler::annotateGroundTruthOfSelection()
{
    Q_ASSERT(!mData->getSelectionMap().isEmpty());

    ClassSelector* dialog = new ClassSelector(w);
    dialog->setWindowTitle("Select class of selection");
    dialog->setSelectedAnnotation(mData->getSelectionMap().first().userAnnotation);

    // connections
    connect(dialog, &ClassSelector::addClass, mData, &MeasurementData::addClass);
    connect(dialog, &ClassSelector::removeClass, mData, &MeasurementData::removeClass);
    connect(dialog, &ClassSelector::changeClass, mData, &MeasurementData::changeClass);

    if (dialog->exec())
    {
        Annotation annotation = dialog->getAnnotation();
        mData->setUserAnnotationOfSelection(annotation);
    }

    dialog->deleteLater();
}

void Controler::annotateDetectionOfSelection()
{
    Q_ASSERT(!mData->getSelectionMap().isEmpty());

    ClassSelector* dialog = new ClassSelector(w);
    dialog->setWindowTitle("[Debug] Select class of detected selection");
    dialog->setSelectedAnnotation(mData->getSelectionMap().first().userAnnotation);

    // connections
    connect(dialog, &ClassSelector::addClass, mData, &MeasurementData::addClass);
    connect(dialog, &ClassSelector::removeClass, mData, &MeasurementData::removeClass);
    connect(dialog, &ClassSelector::changeClass, mData, &MeasurementData::changeClass);

    if (dialog->exec())
    {
        Annotation annotation = dialog->getAnnotation();
        mData->setDetectedAnnotationOfSelection(annotation);
    }

    dialog->deleteLater();
}

void Controler::deleteGroundTruthOfSelection()
{
    Q_ASSERT(!mData->getSelectionMap().isEmpty());

    mData->setUserAnnotationOfSelection(Annotation());
}

void Controler::loadClassifier()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    // load classifier dir
    QString classiferPath = settings.value(CLASSIFIER_DIR_KEY, DEFAULT_CLASSIFIER_DIR).toString();

    // get path to classifier
    QString filename = QFileDialog::getOpenFileName(w, tr("Load smell classifier"), classiferPath, "TorchScript Files (*.pt)");
    if (filename == "")
        return;

    // save classifier path
    QStringList filePath = filename.split("/");
    filePath.removeLast();
    settings.setValue(CLASSIFIER_DIR_KEY, filePath.join("/"));

    // close previous classifier
    if (classifier != nullptr)
        closeClassifier();

    // load new classifier
    bool loadOk;
    QString errorString;
    classifier = new TorchClassifier(this, filename, &loadOk, &errorString);

    // loading classifier failed
    if (!loadOk)
    {
        QMessageBox::critical(w, "Error loading model", errorString);
        closeClassifier();
        return;
    }
    // store changed status to reset after adding new classes
    // loading a classifier should not change the measurement!
    bool prevChanged = mData->isChanged();

    // loading classifier successfull
    // add classifier classes to mData
    for (QString className : classifier->getClassNames())
    {
        aClass c{className};

        if (!aClass::staticClassSet.contains(c))
            mData->addClass(c);
    }
    // restore changed state
    mData->setDataChanged(prevChanged);

    // check classifier func and the functionalisation set
    auto functionalisation = mData->getFunctionalisation();
    auto sensorFailures = mData->getSensorFailures();
    auto funcMap = functionalisation.getFuncMap(sensorFailures);
    if (classifier->getN() != funcMap.size() || classifier->getPresetName() != mData->getFunctionalisation().getName())
    {
        QString error_message = "Functionalisation of the data loaded seems to be incompatible with the loaded classifier.\nWas the functionalisation set correctly? Is the classifier compatible with the sensor used?";
        QMessageBox::warning(w, "Classifier error", error_message);
    }

    QString name = classifier->getName();
    QStringList classNames = classifier->getClassNames();
    bool isInputAbsolute = classifier->getIsInputAbsolute();
    QString presetName =  classifier->getPresetName();

    w->setClassifier(name, classNames, isInputAbsolute, presetName);
}

void Controler::closeClassifier()
{
    delete classifier;
    classifier = nullptr;

    w->closeClassifier();
}

void Controler::classifyMeasurement()
{
    // get measurement data
    QMap<uint, AbsoluteMVector> measDataMap = mData->getAbsoluteData();

    // classify
    for (uint timestamp : measDataMap.keys())
    {
        MVector vector = classifier->getIsInputAbsolute() ? static_cast<MVector>(measDataMap[timestamp]) : static_cast<MVector>(measDataMap[timestamp].getRelativeVector());
        auto funcVector = vector.getFuncVector(mData->getFunctionalisation(), mData->getSensorFailures(), classifier->getInputFunctionType());
        try {
            Annotation annotation = classifier->getAnnotation(funcVector.getVector());
            mData->setDetectedAnnotation(annotation, timestamp);
        } catch (std::invalid_argument& e) {
            QString error_message = e.what() + QString("\nDo you want to close the classifier?");

            QMessageBox::StandardButton answer = QMessageBox::question(w, "Classifier error", error_message);
            if (answer == QMessageBox::StandardButton::Yes)
            {
                closeClassifier();
            }

            break;
        }
    }
}

void Controler::updateAutosave()
{
    if (mData->isChanged() && !mData->getAbsoluteData().isEmpty() && !w->isConverterRunning())
    {
        try {
            mData->saveData(autosavePath + "/" + autosaveName);

            // reset dataChanged (is unset by saveData)
            mData->setDataChanged(true);
        } catch (std::runtime_error e) {
            QMessageBox::warning(w, "Error creating autosave", e.what());
        }
    }
}

void Controler::fitCurves()
{
    CurveFitWizard wizard(mData, w);
    wizard.exec();
}
