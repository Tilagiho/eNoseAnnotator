#include "curvefitwizard.h"
#include <cstdlib>

Q_DECLARE_METATYPE(LeastSquaresFitter::Type);

CurveFitWizard::CurveFitWizard(MeasurementData* mData, QWidget* parent):
    QWizard(parent),
    introPage(new IntroPage),
    fitPage(new FitPage),
    resultPage(new ResultPage(mData)),
    worker(new CurveFitWorker(mData)),
    nChannels(mData->nChannels())
{
    setWindowTitle(tr("Fit curve to selection"));
    setMinimumWidth(1200);
    setMinimumHeight(700);

    setPage(Page_Intro, introPage);
    setPage(Page_Fit, fitPage);
    setPage(Page_Result, resultPage);

    // don't delete QRunnable after run() was finished
    // -> 1 call to run for each channel
    worker->setAutoDelete(false);
//    QThreadPool::globalInstance()->setMaxThreadCount(1);

    // page connections:
    // worker settings
    connect(introPage, &IntroPage::typeChanged, this, &CurveFitWizard::selectType);
    connect(introPage, &IntroPage::detectExpositionChanged, worker, &CurveFitWorker::setDetectExpositionStart);
    connect(introPage, &IntroPage::jumpBaseThresholdChanged, worker, &CurveFitWorker::setJumpBaseThreshold);
    connect(introPage, &IntroPage::jumpFactorChanged, worker, &CurveFitWorker::setJumpFactor);

    connect(introPage, &IntroPage::detectRecoveryChanged, worker, &CurveFitWorker::setDetectRecoveryStart);
    connect(introPage, &IntroPage::recoveryFactorChanged, worker, &CurveFitWorker::setRecoveryFactor);
    connect(introPage, &IntroPage::fitBufferChanged, worker, &CurveFitWorker::setFitBuffer);
    connect(introPage, &IntroPage::recoveryTimeChanged, worker, &CurveFitWorker::setT_recovery);

    connect(introPage, &IntroPage::nIterationsChanged, worker, &CurveFitWorker::setNIterations);
    connect(introPage, &IntroPage::limitFactorChanged, worker, &CurveFitWorker::setLimitFactor);

    // range determination
    connect(worker, &CurveFitWorker::rangeRedeterminationPossible, introPage, &IntroPage::setRangeRedeterminationPossible);
    connect(introPage, &IntroPage::rangeDeterminationRequested, worker, &CurveFitWorker::determineChannelRanges);
    connect(worker, &CurveFitWorker::rangeDeterminationStarted, fitPage, &FitPage::onRangeDeterminationStarted);
    connect(worker, &CurveFitWorker::rangeDeterminationFinished, fitPage, &FitPage::onRangeDeterminationFinished);

    // fit process
    connect(fitPage, &FitPage::fitRequested, this, &CurveFitWizard::fitCurves);
    connect(worker, &CurveFitWorker::started, fitPage, &FitPage::onStarted);
    connect(worker, &CurveFitWorker::finished, fitPage, &FitPage::onFinished);

    qRegisterMetaType<QList<QList<double>>>("QList<QList<double>>");
    connect(worker, &CurveFitWorker::dataSet, resultPage, &ResultPage::setData); // set data in results page

    connect(worker, &CurveFitWorker::progressChanged, fitPage, &FitPage::onProgressChanged);
    connect(worker, &CurveFitWorker::error, fitPage, &FitPage::onError);

    // result connections
    connect(resultPage, &ResultPage::saveResultsRequested, this, &CurveFitWizard::saveData); // save fit results
    connect(resultPage, &ResultPage::channelRangeRequested, worker, &CurveFitWorker::emitChannelRange);
    connect(worker, &CurveFitWorker::channelRangeProvided, resultPage, &ResultPage::setChannelRange);

    connect(resultPage, &ResultPage::removeFromRange, worker, &CurveFitWorker::removeFromRange);
    connect(resultPage, &ResultPage::addToRange, worker, &CurveFitWorker::addToRange);

    connect(worker, &CurveFitWorker::rangeDeterminationFinished, this, &CurveFitWizard::updateChannelRange);
}

CurveFitWizard::~CurveFitWizard()
{
    // stop worker threads and delete worker
    QThreadPool::globalInstance()->clear();
    QThreadPool::globalInstance()->waitForDone();
    worker->deleteLater();
}

void CurveFitWizard::selectType()
{
    auto type = LeastSquaresFitter::getTypeMap()[field("typeSelector").toString()];
    worker->setType(type);
}

void CurveFitWizard::saveData()
{
    QString filePath = QFileDialog::getSaveFileName(this,
         tr("Save curve fit results"), "",
         tr("Data files (*.csv);;All Files (*)"));

    if (!filePath.isEmpty())
    {
        if (!filePath.endsWith(".csv"))
            filePath += ".csv";
        worker->save(filePath);
        resultPage->setDataSaved(true);
    }
}

void CurveFitWizard::updateChannelRange()
{
    int channel = resultPage->getCurrentChannel();

    if (channel >= 0)
        worker->emitChannelRange(channel);
}

void CurveFitWizard::fitCurves()
{
    worker->init();

    // debug: only one active fitWorker thread at once
    if (CVWIZ_DEBUG_MODE)
        QThreadPool::globalInstance()->setMaxThreadCount(1);

    qDebug() << "\n--------\nStarting curve fit:";
    qDebug() << "max thread count:\t" << QString::number(QThreadPool::globalInstance()->maxThreadCount());
    for (size_t i=0; i<nChannels; i++)
        QThreadPool::globalInstance()->start(worker);
}

IntroPage::IntroPage(QWidget* parent):
    QWizardPage(parent),
    detectionLayout(new QFormLayout),
    typeSelector(new QComboBox),
    detectExpositionStartCheckBox(new QCheckBox),
    detectRecoveryCheckBox(new QCheckBox),
    limitFactorSpinBox(new QDoubleSpinBox),
    jumpFactorSpinBox(new QDoubleSpinBox),
    jumpBaseThresholdSpinBox(new QDoubleSpinBox),
    recoveryFactorSpinBox(new QDoubleSpinBox),
    nIterationsSpinBox(new QSpinBox),
    fitBufferSpinBox(new QSpinBox),
    recoveryTimeSpinBox(new QSpinBox)
{
    setTitle("Settings");

    auto modelTypeMap = LeastSquaresFitter::getTypeMap();

    // add to comboBox
    for (QString modelString : modelTypeMap.keys())
        typeSelector->addItem(modelString);
    LeastSquaresFitter::Type defaultType = CVWIZ_DEFAULT_MODEL_TYPE;
    int index = modelTypeMap.values().indexOf(defaultType);
    typeSelector->setCurrentText(modelTypeMap.keys()[index]);

    QGroupBox *modelGroupBox = new QGroupBox("Model settings");
    QFormLayout *modelLayout = new QFormLayout;
    modelLayout->addRow(tr("Model:"), typeSelector);

    nIterationsSpinBox->setRange(1, 1000);
    nIterationsSpinBox->setValue(LEAST_SQUARES_N_ITERATIONS);
    modelLayout->addRow("Iterations", nIterationsSpinBox);
    modelLayout->labelForField(nIterationsSpinBox)->setToolTip("Number of solving repetitions for each channel");

    limitFactorSpinBox->setRange(1.0, 1000.);
    limitFactorSpinBox->setSingleStep(0.1);
    limitFactorSpinBox->setValue(LEAST_SQUARES_LIMIT_FACTOR);
    modelLayout->addRow("Conversion limit factor", limitFactorSpinBox);
    modelLayout->labelForField(limitFactorSpinBox)->setToolTip("The convergion limit defines the maximum convergion value of accepted solutions.\nConvergion limit = convergion limit factor * maxValue(channel)");

    modelGroupBox->setLayout(modelLayout);

    QGroupBox *detectiongroupBox = new QGroupBox(tr("Detection settings"));
    detectiongroupBox->setLayout(detectionLayout);

    detectExpositionStartCheckBox->setCheckState(CVWIZ_DEFAULT_DETECT_EXPOSITION_START ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    detectionLayout->addRow("Detect start of exposition", detectExpositionStartCheckBox);

    jumpBaseThresholdSpinBox->setMinimum(0.0);
    jumpBaseThresholdSpinBox->setSingleStep(0.1);
    jumpBaseThresholdSpinBox->setValue(CVWIZ_DEFAULT_JUMP_BASE_THRESHOLD);
    detectionLayout->addRow("Jump base threshold", jumpBaseThresholdSpinBox);
    detectionLayout->labelForField(jumpBaseThresholdSpinBox)->setEnabled(CVWIZ_DEFAULT_DETECT_EXPOSITION_START);

    jumpFactorSpinBox->setMinimum(0.0);
    jumpFactorSpinBox->setSingleStep(1.0);
    jumpFactorSpinBox->setValue(CVWIZ_DEFAULT_JUMP_FACTOR);
    detectionLayout->addRow("Jump factor", jumpFactorSpinBox);
    detectionLayout->labelForField(jumpFactorSpinBox)->setEnabled(CVWIZ_DEFAULT_DETECT_EXPOSITION_START);

    detectRecoveryCheckBox->setCheckState(CVWIZ_DEFAULT_DETECT_RECOVERY_START ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    detectionLayout->addRow("Detect end of exposition", detectRecoveryCheckBox);

    recoveryFactorSpinBox->setMinimum(0.1);
    recoveryFactorSpinBox->setSingleStep(0.1);
    recoveryFactorSpinBox->setValue(CVWIZ_DEFAULT_RECOVERY_FACTOR);
    recoveryFactorSpinBox->setEnabled(CVWIZ_DEFAULT_DETECT_RECOVERY_START);
    detectionLayout->addRow("Recovery threshold", recoveryFactorSpinBox);
    detectionLayout->labelForField(recoveryFactorSpinBox)->setEnabled(CVWIZ_DEFAULT_DETECT_RECOVERY_START);

    fitBufferSpinBox->setMinimum(5);
    fitBufferSpinBox->setSuffix("s");
    fitBufferSpinBox->setValue(CVWIZ_DEFAULT_BUFFER_SIZE);
    detectionLayout->addRow("Buffer length", fitBufferSpinBox);

    recoveryTimeSpinBox->setSuffix("min");
    recoveryTimeSpinBox->setValue(CVWIZ_DEFAULT_RECOVERY_TIME);
    detectionLayout->addRow("Max recovery time", recoveryTimeSpinBox);

    QVBoxLayout *windowLayout = new QVBoxLayout;
    windowLayout->addWidget(modelGroupBox);
    windowLayout->addWidget(detectiongroupBox);
    setLayout(windowLayout);

    connect(typeSelector, &QComboBox::currentTextChanged, this, &IntroPage::typeChanged);
    connect(nIterationsSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(nIterationsChanged(int)));
    connect(limitFactorSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(limitFactorChanged(double)));

    connect(jumpBaseThresholdSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(jumpBaseThresholdChanged(double)));
    connect(jumpFactorSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(jumpFactorChanged(double)));
    connect(recoveryFactorSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(recoveryFactorChanged(double)));
    connect(fitBufferSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(fitBufferChanged(int)));
    connect(recoveryTimeSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(recoveryTimeChanged(int)));

    connect(detectExpositionStartCheckBox, &QCheckBox::clicked, this, &IntroPage::setDetectExpositionStart);

    connect(detectRecoveryCheckBox, &QCheckBox::clicked, this, &IntroPage::setDetectRecoveryStart);
}

bool IntroPage::validatePage()
{
    if (rangeRedeterminationPossible)
    {
        QMessageBox mBox;
        mBox.setText("The range determination settings were changed since the last range determination or the ranges were changed manually.\n"
                     "Do you want to redetermine the ranges with the current settings?");
        mBox.setStandardButtons({QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel});
        auto ans = mBox.exec();

        if (ans == QMessageBox::Cancel)
            return false;
        else if (ans == QMessageBox::No)
            return true;
    }

    emit rangeDeterminationRequested();
    rangeRedeterminationPossible = false;
    return true;
}

IntroPage::~IntroPage()
{
}

void IntroPage::setRangeRedeterminationPossible()
{
    rangeRedeterminationPossible = true;
}

void IntroPage::setDetectExpositionStart(bool value)
{
    jumpBaseThresholdSpinBox->setEnabled(value);
    detectionLayout->labelForField(jumpBaseThresholdSpinBox)->setEnabled(value);
    jumpFactorSpinBox->setEnabled(value);
    detectionLayout->labelForField(jumpFactorSpinBox)->setEnabled(value);

    detectRecoveryCheckBox->setEnabled(value);
    detectionLayout->labelForField(detectRecoveryCheckBox)->setEnabled(value);
    if (!value || detectRecoveryCheckBox->isChecked())
        setDetectRecoveryStart(value);

    emit detectExpositionChanged(value);
}

void IntroPage::setDetectRecoveryStart(bool value)
{
    recoveryFactorSpinBox->setEnabled(value);
    detectionLayout->labelForField(recoveryFactorSpinBox)->setEnabled(value);
    emit detectRecoveryChanged(value);
}


FitPage::FitPage(QWidget *parent):
    QWizardPage(parent),
    progressbar(new QProgressBar),
    startButton(new QPushButton)
{
    // page layout
    setTitle("Fit curves");

    progressbar->setMinimum(0);
    progressbar->setMaximum(100);

    startButton->setText("Fit curves");
    connect(startButton, &QPushButton::clicked, this, &FitPage::startFit);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(progressbar);
    QHBoxLayout *hLayout = new QHBoxLayout;
    QSpacerItem *spacer = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hLayout->addItem(spacer);
    hLayout->addWidget(startButton);
    layout->addLayout(hLayout);

    setLayout(layout);
}

FitPage::~FitPage(){

}

bool FitPage::isComplete() const
{
    return progressbar->value() == 100;
}

void FitPage::startFit()
{
    // update window
    setTitle("Fitting curves...");

    progressbar->setValue(0);

    for(QWizard::WizardButton which: {QWizard::BackButton, QWizard::NextButton})
        if(QAbstractButton * button = wizard()->button(which))
            button->setEnabled(false);

    startButton->setEnabled(false);

    emit fitRequested();
}

void FitPage::onStarted()
{

}

void FitPage::onFinished()
{
    setTitle("Curves fitted");

    progressbar->setValue(100);
    startButton->setText("Refit curves");

    for(QWizard::WizardButton which : {QWizard::BackButton, QWizard::NextButton, QWizard::CancelButton, QWizard::FinishButton})
        if(QAbstractButton * button = wizard()->button(which))
            button->setEnabled(true);

    emit completeChanged();
}

void FitPage::onProgressChanged(int value)
{
    progressbar->setValue(qRound((100.0 ) * (value + 1)/ MVector::nChannels));
}

void FitPage::onError(QString errorMessage)
{
    QMessageBox* box = new QMessageBox();
    box->setIcon(QMessageBox::Icon::Warning);
    box->setWindowTitle("Fitting error");
    box->setText(errorMessage);
}

void FitPage::onRangeDeterminationStarted()
{
    startButton->setEnabled(false);
}

void FitPage::onRangeDeterminationFinished()
{
    startButton->setEnabled(true);
}

ResultPage::ResultPage(MeasurementData *mData, QWidget *parent):
    QWizardPage(parent),
    resultTable(new QTableWidget),
    rangeTable(new QTableWidget),
    resultBox(new QGroupBox("Fit results")),
    channelDataBox(new QGroupBox("Data channel 1")),
    minusButton(new QPushButton),
    addButton(new QPushButton),
    saveButton(new QPushButton("Save...")),
    mData(mData)
{   
    setTitle("Results");

    // set behaviour of results table
    resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultTable->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    resultTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    resultTable->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    resultTable->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // init range table
    selectedData = mData->getSelectionMap();
    for (auto timestamp : selectedData.keys())
        selectedData[timestamp] = selectedData[timestamp].getRelativeVector();
    rangeTable->setRowCount(1);
    rangeTable->setColumnCount(selectedData.size());
    rangeTable->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    rangeTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectColumns);
    rangeTable->setVerticalHeaderLabels({"y"});
    rangeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    rangeTable->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));

    QStringList header;
    auto startTimestamp = mData->getStartTimestamp();
    for (int index=0; index < selectedData.size(); index++)
    {
        auto timestamp = selectedData.keys()[index];
        auto elapsedTime = timestamp - startTimestamp;
        QString elapsedTimeString;
        if (elapsedTime / 3600 > 0)
            elapsedTimeString = QString("%1:%2:%3")
                          .arg( elapsedTime / 3600, 2, 10, QChar('0'))
                          .arg( (elapsedTime % 3600) / 60, 2, 10, QChar('0'))
                          .arg(elapsedTime % 60, 2, 10, QChar('0'));
        else
            elapsedTimeString = QString("%1:%2")
                          .arg( elapsedTime / 60, 2, 10, QChar('0'))
                          .arg(elapsedTime % 60, 2, 10, QChar('0'));

        header << elapsedTimeString;
    }
    rangeTable->setHorizontalHeaderLabels(header);

    // setup layout of the page:
    QVBoxLayout* windowLayout = new QVBoxLayout;

    // result box
    QVBoxLayout* resultLayout = new QVBoxLayout;
    resultLayout->addWidget(resultTable);

    QHBoxLayout* hLayout = new QHBoxLayout;
    QSpacerItem *spacer = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hLayout->addItem(spacer);
    hLayout->addWidget(saveButton);
    resultLayout->addLayout(hLayout);

    resultBox->setLayout(resultLayout);

    // channel data
    auto *channelDataLayout = new QVBoxLayout;
    channelDataLayout->addWidget(rangeTable);
    auto *buttonLayout = new QHBoxLayout;
    auto buttonSpacer = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
    addButton->setIcon(QIcon(":/icons/add"));
    addButton->setEnabled(false);
    addButton->setToolTip("Add selection to channel data range");
    minusButton->setIcon(QIcon(":/icons/minus"));
    minusButton->setEnabled(false);
    minusButton->setToolTip("Remove selection from channel data range");
    buttonLayout->addItem(buttonSpacer);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(minusButton);
    channelDataLayout->addLayout(buttonLayout);

    channelDataBox->setLayout(channelDataLayout);
    // DEBUG
    channelDataBox->hide();
//    Spoiler *spoiler = new Spoiler("Channel data");
//    spoiler->setContentLayout(*spoilerLayout);
//    spoiler->setIsFoldedOut(true);
//    spoiler->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));

    windowLayout->addWidget(resultBox);
    windowLayout->addWidget(channelDataBox);

//    layout->setAlignment(resultTable, Qt::AlignHCenter);
    setLayout(windowLayout);

    // connect signals
    connect(saveButton, &QPushButton::clicked, this, &ResultPage::saveResultsRequested); // save results

    connect(resultTable, &QTableWidget::itemSelectionChanged, this, &ResultPage::resultSelectionChanged); // request update of range

    connect(rangeTable, &QTableWidget::itemSelectionChanged, this, &ResultPage::channelDataSelectionChanged);

    connect(minusButton, &QPushButton::clicked, this, &ResultPage::requestRangeRemoval);

    connect(addButton, &QPushButton::clicked, this, &ResultPage::requestRangeExtension);
}

ResultPage::~ResultPage()
{
}

bool ResultPage::isComplete() const
{
    return dataSaved;
}

void ResultPage::setData(QStringList header, QStringList tooltips, QList<QList<double>> data)
{
    Q_ASSERT(header.size() == tooltips.size());
    Q_ASSERT(header.size() == data.size());

    resultTable->setRowCount(MVector::nChannels);
    resultTable->setColumnCount(data.size());

    // set header
    resultTable->setHorizontalHeaderLabels(header);


    // set values & tooltips
    auto sensorFailures = mData->getSensorFailures();
    for (int column=0; column<data.size(); column++)
    {
        // set tooltip
        resultTable->horizontalHeaderItem(column)->setToolTip(tooltips[column]);

        // set values
        for (int row=0; row<MVector::nChannels; row++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(data[column][row], 'g', 5));

            if (sensorFailures[row])
                item->setBackgroundColor(Qt::gray);

            resultTable->setItem(row, column, item);
        }
    }

//    resultTable->resizeColumnsToContents();

    // adjust width of table
//    int w = resultTable->verticalHeader()->width() + resultTable->horizontalHeader()->length() + 2*resultTable->frameWidth() + resultTable->verticalScrollBar()->width();
//    resultTable->setFixedWidth(w);
    resultTable->verticalScrollBar()->setHidden(false);
    resultTable->selectRow(0);
}

void ResultPage::setChannelRange(int channel,QList<uint> channelRange)
{
    auto keys = selectedData.keys();

    for (int i=0; i<selectedData.size(); i++)
    {
        auto timestamp = keys[i];
        QTableWidgetItem *item = new QTableWidgetItem;

        item->setText(QString::number(selectedData[timestamp][channel], 'g', 3));

        if (channelRange.contains(timestamp))
            item->setBackgroundColor(Qt::blue);

        rangeTable->setItem(0, i, item);
    }

    rangeTable->resizeColumnsToContents();
    int h = rangeTable->verticalHeader()->length() + rangeTable->horizontalHeader()->height() + 2 * rangeTable->frameWidth() + rangeTable->horizontalScrollBar()->height();
    rangeTable->setFixedHeight(h+1);

    // set groupBox title
    channelDataBox->setTitle("Data channel " + QString::number(channel+1));
}
void ResultPage::resultSelectionChanged()
{
    emit channelRangeRequested(resultTable->currentRow());
}

void ResultPage::channelDataSelectionChanged()
{
    if (rangeTable->selectedItems().empty())
    {
        minusButton->setEnabled(false);
        addButton->setEnabled(false);
    } else
    {
        minusButton->setEnabled(true);
        addButton->setEnabled(true);
    }
}

void ResultPage::requestRangeRemoval()
{
    int channel = resultTable->currentRow();
    QList<int> rangeList;
    for (auto item : rangeTable->selectedItems())
        rangeList << item->column();

    emit removeFromRange(channel, rangeList);
}

void ResultPage::requestRangeExtension()
{
    int channel = resultTable->currentRow();
    QList<int> rangeList;
    for (auto item : rangeTable->selectedItems())
        rangeList << item->column();

    emit addToRange(channel, rangeList);
}

void ResultPage::setDataSaved(bool value)
{
    dataSaved = value;
    emit completeChanged();
}

int ResultPage::getCurrentChannel()
{
    return resultTable->currentRow();
}
