#include "convertwizard.h"

#include <QtCore>
#include <QtWidgets>
#include <QFileDialog>

#include "../classes/measurementdata.h"
#include "functionalisationdialog.h"

ConvertWizard::ConvertWizard(QWidget* parent):
    QWizard(parent)
{
    setWindowTitle(tr("eNoseAnnotator Converter"));

    addPage(createIntroPage());
    addPage(new FileSelectionPage);
    addPage(new ConversionPage);
}

ConvertWizard::~ConvertWizard()
{}

QWizardPage* ConvertWizard::createIntroPage()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle("Introduction");

    QLabel *label = new QLabel("You can use the eNoseAnnotator Converter tool to convert raw measurement files to the "
                               "eNoseAnnotator format.\n"
                               "Currently supports the Leif format.");
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

/*!
 * \brief FileSelectionPage::FileSelectionPage QWizardPage for setting source filenames, targetDir & functionalisation for the conversion.
 * \param parent
 */
FileSelectionPage::FileSelectionPage(QWidget *parent):
    QWizardPage(parent)
{
    setTitle("Select files to be converted and target directory");
    setCommitPage(true);
    setButtonText(QWizard::WizardButton::CommitButton, "Convert");

    // row 1: source files
    sourceFilesInfoLabel = new QLabel("Source files:");
    sourceFilesLineEdit = new QLineEdit("");
    sourceFilesLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sourceFilesButton = new QPushButton("...");
    sourceFilesButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    // row 2: target dir
    targetDirInfoLabel = new QLabel("Target dir:");
    targetDirLineEdit = new QLineEdit("export/");
    targetDirLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    targetDirButton = new QPushButton("...");
    targetDirButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    // row 3: sensor id
    sensorInfoLabel = new QLabel("Sensor ID:");
    sensorIDLineEdit = new QLineEdit();
    sensorIDLineEdit->setText("default");

    // row 4: nChannels
    nChannelsInfoLabel = new QLabel("Number of channels:");
    nChannelsSpinBox = new QSpinBox();
    nChannelsSpinBox->setValue(MVector::nChannels);

    // row 4: functionalisation
    funcInfoLabel = new QLabel("Functionalisation:");
    funcLabel = new QLabel("None");
    funcButton = new QPushButton("...");

    // connect line edits to signal for checking isComplete()
    connect(sourceFilesLineEdit, &QLineEdit::textChanged, this, [this](QString){
        emit completeChanged();
    });
    connect(targetDirLineEdit, &QLineEdit::textChanged, this, [this](QString){
        emit completeChanged();
    });

    // connect widgets to dialogs
    connect(sourceFilesButton, &QPushButton::clicked, this, &FileSelectionPage::getSourceFiles);
    connect(targetDirButton, &QPushButton::clicked, this, &FileSelectionPage::getTargetDir);
    connect(funcButton, &QPushButton::clicked, this, &FileSelectionPage::getFuncs);

    // store values for other pages
    registerField("sourceFilenames", sourceFilesLineEdit);
    registerField("targetDir", targetDirLineEdit);
    registerField("functionalisation", this, "functionalisations");
    registerField("nChannels", nChannelsSpinBox);
    registerField("sensorId", sensorIDLineEdit);


    // layout widgets
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(sourceFilesInfoLabel, 0, 0);
    layout->addWidget(sourceFilesLineEdit, 0, 1);
    layout->addWidget(sourceFilesButton, 0, 2);

    layout->addWidget(targetDirInfoLabel, 1, 0);
    layout->addWidget(targetDirLineEdit, 1, 1);
    layout->addWidget(targetDirButton, 1, 2);

    layout->addWidget(sensorInfoLabel, 2, 0);
    layout->addWidget(sensorIDLineEdit, 2, 1);

    layout->addWidget(nChannelsInfoLabel, 3, 0);
    layout->addWidget(nChannelsSpinBox, 3, 1);

    layout->addWidget(funcInfoLabel, 4, 0);
    layout->addWidget(funcLabel, 4, 1);
    layout->addWidget(funcButton, 4, 2);

    setLayout(layout);
}

/*!
 * \brief FileSelectionPage::getSourceFiles opens dialog for the input filenames & sets sourceFilesLabel to the result.
 * filenames in sourceFilesLabel are separated by ';'
 */
void FileSelectionPage::getSourceFiles()
{
    // get current dir of source files
    QStringList sourceFilenames = QFileDialog::getOpenFileNames(this, "Select raw measurement files");

    if (!sourceFilenames.isEmpty())
    {
        QString filenamesString = sourceFilenames.join(';');
        sourceFilesLineEdit->setText(filenamesString);
    }
}

/*!
 * \brief FileSelectionPage::getTargetDir opens dialog for the target directory & sets targetDirLabel to the result
 */
void FileSelectionPage::getTargetDir()
{
    QString targetDir = QFileDialog::getExistingDirectory(this, "Select save directory", targetDirLineEdit->text());

    if (!targetDir.isEmpty())
    {
        targetDirLineEdit->setText(targetDir);
    }
}

/*!
 * \brief FileSelectionPage::getFuncs opens dialog for choosing the functionalisation of the raw input files & sets funcLabel to the preset name
 */
void FileSelectionPage::getFuncs()
{
    FunctionalisationDialog dialog;

    if (dialog.exec())
    {
        functionalisations = dialog.getFunctionalisations();
        funcLabel->setText(dialog.presetName);
    }
}

/*!
 * \brief FileSelectionPage::validatePage
 * \return
 */
bool FileSelectionPage::validatePage()
{
    // reset label colors
    sourceFilesInfoLabel->setStyleSheet("QLabel {color: black}");
    targetDirInfoLabel->setStyleSheet("QLabel {color: black}");
    nChannelsInfoLabel->setStyleSheet("QLabel {color: black}");
    funcInfoLabel->setStyleSheet("QLabel {color: black}");
    sensorInfoLabel->setStyleSheet("QLabel {color: black}");

    bool isValid = true;

    // check existence of source filenames
    QStringList sourceFilenames = sourceFilesLineEdit->text().split(";");
    for (QString filename : sourceFilenames)
    {
        if (!QFile(filename).exists())
        {
            sourceFilesInfoLabel->setStyleSheet("QLabel {color: red}");
            isValid = false;
        }
    }

    // check existence of target dir
    QString targetDir = targetDirLineEdit->text();

    if (!QDir(targetDir).exists())
    {
        targetDirInfoLabel->setStyleSheet("QLabel {color: red}");
        isValid = false;
    }

    if (functionalisations.empty())
        functionalisations = std::vector<int> (nChannelsSpinBox->value(), 0);
    else if (functionalisations.size() != nChannelsSpinBox->value())
    {
        isValid = false;
        nChannelsInfoLabel->setStyleSheet("QLabel {color: red}");
        funcInfoLabel->setStyleSheet("QLabel {color: red}");
    }

    if (sensorIDLineEdit->text().isEmpty())
    {
        isValid = false;
        sensorInfoLabel->setStyleSheet("QLabel {color: red}");
    }

    return isValid;
}

bool FileSelectionPage::isComplete() const
{
    bool sourceFilenamesSet = !sourceFilesLineEdit->text().isEmpty();
    bool targetDirSet = !targetDirLineEdit->text().isEmpty();

    return sourceFilenamesSet && targetDirSet;
}

ConversionPage::ConversionPage(QWidget *parent):
    QWizardPage(parent),
    worker(),
    label(new QLabel),
    progressbar(new QProgressBar)
{
    // page layout
    setTitle("Converting files");
//    setFinalPage(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(progressbar);

    progressbar->setMinimum(0);
    progressbar->setMaximum(100);

    setLayout(layout);

    // prepare coversion thread
    thread = new QThread(this);
    worker.moveToThread(thread);
    connect(&worker, &ConvertWorker::started, this, &ConversionPage::onStarted);
    connect(&worker, &ConvertWorker::finished, this, &ConversionPage::onFinished);
    connect(&worker, &ConvertWorker::progressChanged, this, &ConversionPage::onProgressChanged);
    connect(&worker, &ConvertWorker::error, this, &ConversionPage::onError);
    thread->start();


}
ConversionPage::~ConversionPage(){
    thread->quit();
    thread->wait();
}

void ConversionPage::initializePage()
{
    startConversion();
}

void ConversionPage::startConversion()
{
    // get saved variables
    filenames = qvariant_cast<QString> (field("sourceFilenames")).split(';');
    filenames.removeAll("");

    targetDir = qvariant_cast<QString> (field("targetDir"));
    functionalisation = qvariant_cast<std::vector<int>>(field("functionalisation"));

    QMetaObject::invokeMethod(&worker, "convert", Qt::QueuedConnection, Q_ARG(QStringList, filenames), Q_ARG(QString, targetDir), Q_ARG(std::vector<int>, functionalisation));
}

void ConversionPage::onStarted()
{
    for(QWizard::WizardButton which: {QWizard::BackButton, QWizard::NextButton, QWizard::CancelButton})
        if(QAbstractButton * button = wizard()->button(which))
            button->setEnabled(false);
}

void ConversionPage::onFinished()
{
    for(QWizard::WizardButton which: {QWizard::NextButton})
        if(QAbstractButton * button = wizard()->button(which))
            button->setEnabled(true);

    // restore MVector::nChannels
    MVector::nChannels = nChannels;
}

void ConversionPage::onProgressChanged(int value)
{
    progressbar->setValue(qRound((100.0 / filenames.size()) * (value + 1)));
    label->setText(QFileInfo(filenames[value]).fileName());
}

void ConversionPage::onError(QString errorMessage)
{
    QMessageBox* box = new QMessageBox();
    box->setIcon(QMessageBox::Icon::Warning);
    box->setWindowTitle("Conversion error");
    box->setText(errorMessage + "\n\nDo you want to skip this file to continue the conversion?");
    box->setStandardButtons(QMessageBox::Ignore| QMessageBox::Abort);
    auto buttonY = box->button(QMessageBox::Ignore);
    buttonY->setText("Skip");
    auto buttonN = box->button(QMessageBox::Abort);
    buttonN->setText("Cancel");
    int ans = box->exec();

    switch (ans) {
    case QMessageBox::StandardButton::Ignore:
        worker.resume();
        break;
    case QMessageBox::StandardButton::Abort:
        worker.cancel();
        break;
    }
}

void ConvertWorker::convert(const QStringList sourceFilenames, const QString targetDir, const std::vector<int> functionalisation)
{
    Q_EMIT started();
    for(int i=0; i<sourceFilenames.size(); i++)
    {
        QString filename = sourceFilenames[i];
        // convert file
        try
        {
            convertFile(filename, targetDir, functionalisation);
        }
        // on error: emit error and wait until resume() slot is called
        catch (std::runtime_error e)
        {
            emit error(e.what());
            sync.lock();
            pauseCond.wait(&sync);
            sync.unlock();
        }
        Q_EMIT progressChanged(i);
    }
//    qDebug()<< __PRETTY_FUNCTION__ << "finished";
    Q_EMIT finished();
}

void ConvertWorker::convertFile(QString filename, QString targetDir, std::vector<int> functionalisation)
{
    FileReader generalReader(filename);
    FileReader* specificReader = generalReader.getSpecificReader();

    // check type of specificFileReader
    switch (specificReader->getType()) {
    case FileReader::FileReaderType::Leif:
        break;
    default:
        throw std::runtime_error("Cannot convert " + QFileInfo(filename).fileName().toStdString());
    }

    MeasurementData* data = specificReader->getMeasurementData();

    if (MVector::nChannels != functionalisation.size())
        throw std::runtime_error("Error converting file " + QFileInfo(filename).fileName().toStdString() + "\nFunctionalisation incompatible with number of channels!");

    data->setFunctionalisation(functionalisation);

    QFileInfo fileInfo(filename);
    QString targetFilename = targetDir + "/" + fileInfo.fileName();
    try {
        data->saveData(targetFilename);
    } catch (std::runtime_error e) {
        emit error(e.what());
    }

    delete specificReader;
}

void ConvertWorker::resume()
{
    pauseCond.wakeAll();
}

void ConvertWorker::cancel()
{
    // TODO
}
