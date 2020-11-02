#include "curvefitwizard.h"
#include <cstdlib>

Q_DECLARE_METATYPE(LeastSquaresFitter::Type);

CurveFitWizard::CurveFitWizard(MeasurementData* mData, QWidget* parent):
    QWizard(parent),
    introPage(new IntroPage),
    fitPage(new FitPage),
    resultPage(new ResultPage(mData)),
    worker(mData),
    thread(new QThread)
{
    setWindowTitle(tr("Fit curve to selection"));
    setMinimumWidth(1200);
    setMinimumHeight(700);

    setPage(Page_Intro, introPage);
    setPage(Page_Fit, fitPage);
    setPage(Page_Result, resultPage);

    // setup worker thread
    worker.moveToThread(thread);
    thread->start();

    // page connections:
    // worker settings
    connect(introPage, &IntroPage::typeChanged, this, &CurveFitWizard::selectType);
    connect(introPage, &IntroPage::detectExpositionChanged, &worker, &FitWorker::setDetectExpositionStart);
    connect(introPage, &IntroPage::jumpBaseThresholdChanged, &worker, &FitWorker::setJumpBaseThreshold);
    connect(introPage, &IntroPage::jumpFactorChanged, &worker, &FitWorker::setJumpFactor);

    connect(introPage, &IntroPage::detectRecoveryChanged, &worker, &FitWorker::setDetectRecoveryStart);
    connect(introPage, &IntroPage::recoveryFactorChanged, &worker, &FitWorker::setRecoveryFactor);
    connect(introPage, &IntroPage::fitBufferChanged, &worker, &FitWorker::setFitBuffer);

    connect(introPage, &IntroPage::nIterationsChanged, &worker, &FitWorker::setNIterations);
    connect(introPage, &IntroPage::limitFactorChanged, &worker, &FitWorker::setLimitFactor);

    // range determination
    connect(&worker, &FitWorker::rangeRedeterminationPossible, introPage, &IntroPage::setRangeRedeterminationPossible);
    connect(introPage, &IntroPage::rangeDeterminationRequested, &worker, &FitWorker::determineChannelRanges);
    connect(&worker, &FitWorker::rangeDeterminationStarted, fitPage, &FitPage::onRangeDeterminationStarted);
    connect(&worker, &FitWorker::rangeDeterminationFinished, fitPage, &FitPage::onRangeDeterminationFinished);

    // fit process
    connect(fitPage, &FitPage::fitRequested, &worker, &FitWorker::fit);
    connect(&worker, &FitWorker::started, fitPage, &FitPage::onStarted);
    connect(&worker, &FitWorker::finished, fitPage, &FitPage::onFinished);

    qRegisterMetaType<QList<QList<double>>>("QList<QList<double>>");
    connect(&worker, &FitWorker::dataSet, resultPage, &ResultPage::setData); // set data in results page

    connect(&worker, &FitWorker::progressChanged, fitPage, &FitPage::onProgressChanged);
    connect(&worker, &FitWorker::error, fitPage, &FitPage::onError);

    // result connections
    connect(resultPage, &ResultPage::saveResultsRequested, this, &CurveFitWizard::saveData); // save fit results
    connect(resultPage, &ResultPage::channelRangeRequested, &worker, &FitWorker::emitChannelRange);
    connect(&worker, &FitWorker::channelRangeProvided, resultPage, &ResultPage::setChannelRange);

    connect(resultPage, &ResultPage::removeFromRange, &worker, &FitWorker::removeFromRange);
    connect(resultPage, &ResultPage::addToRange, &worker, &FitWorker::addToRange);

    connect(&worker, &FitWorker::rangeDeterminationFinished, this, &CurveFitWizard::updateChannelRange);
}

CurveFitWizard::~CurveFitWizard()
{
    thread->quit();
    thread->wait();
    thread->deleteLater();
}

void CurveFitWizard::selectType()
{
    auto type = LeastSquaresFitter::getTypeMap()[field("typeSelector").toString()];
    QMetaObject::invokeMethod(&worker, "setType", Qt::QueuedConnection, Q_ARG(LeastSquaresFitter::Type, type));
}

void CurveFitWizard::saveData()
{
    QString fileName = QFileDialog::getSaveFileName(this,
         tr("Save curve fit results"), "",
         tr("Data files (*.csv);;All Files (*)"));

    if (!fileName.isEmpty())
    {
        if (!fileName.endsWith(".csv"))
            fileName += ".csv";
        QMetaObject::invokeMethod(&worker, "save", Qt::QueuedConnection, Q_ARG(QString, fileName));
    }
}

void CurveFitWizard::updateChannelRange()
{
    int channel = resultPage->getCurrentChannel();

    if (channel >= 0)
        QMetaObject::invokeMethod(&worker, "emitChannelRange", Qt::QueuedConnection, Q_ARG(int, channel));
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
    fitBufferSpinBox(new QSpinBox)
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

FitWorker::FitWorker(MeasurementData* mData, QObject *parent):
    QObject(parent),
    sigmaError(MVector::nChannels, 0.),
    tau90(MVector::nChannels, 0.),
    f_t90(MVector::nChannels, 0.),
    sigmaNoise(MVector::nChannels, 0.),
    nSamples(MVector::nChannels, 0),
    mData(mData),
    dataRange(MVector::nChannels, std::vector<std::pair<double, double>>()),
    y_offset(MVector::nChannels, 0),
    x_start(MVector::nChannels, mData->getSelectionMap().firstKey())
{
    selectedData = mData->getSelectionMap();
    auto relativeData = mData->getRelativeData();

    Q_ASSERT(!relativeData.isEmpty());
    Q_ASSERT(!selectedData.isEmpty());

    // convert selectedData to relative
    for (uint timestamp : selectedData.keys())
        selectedData[timestamp] = relativeData[timestamp];
}

void FitWorker::determineChannelRanges()
{
    emit rangeDeterminationStarted();

    for (int i=0; i<MVector::nChannels; i++)
        dataRange[i].clear();

    auto relativeData = mData->getRelativeData();
    auto sensorFailures = mData->getSensorFailures();

    std::vector<uint> x_end(MVector::nChannels, selectedData.lastKey());
    for (int channel=0; channel<MVector::nChannels; channel++)
    {
        // ignore channels with sensor failure flags
        if (sensorFailures[channel])
            continue;

        // adjust data range for channel
        auto it = relativeData.find(selectedData.firstKey());
        auto endIt = relativeData.constFind(selectedData.lastKey());

        bool inRange = false;
        bool reactionIsPositive = true;

        // exposition start detection turned off:
        if (!detectExpositionStart)
        {
            x_start[channel] = selectedData.firstKey();
            y_offset[channel] = selectedData.first()[channel];
            inRange = true;
        }

        // collect points for linear fit:
        // before exposition start:
        // -> fitBuffer seconds before current point
        // after exposition start:
        // -> fitBuffer seconds after current point
        uint x_0 = it.key();
        while(it.key() <= endIt.key() && it != relativeData.constEnd())
        {
            // collect vectors in range [innerIt.key(); innerIt.key() + CURVE_FIT_CHANNEL_BUFFER]
            std::vector<double> x, y;
            auto lineIt = it;

            while(std::labs(static_cast<long>(lineIt.key()) - static_cast<long>(it.key())) < fitBuffer)
            {
                x.push_back(lineIt.key() - x_0);
                y.push_back(lineIt.value()[channel]);

                // in range:
                // fit line to subsequent values
                if (inRange)
                    lineIt++;
                // not in range:
                // fit line to previous values
                else
                    lineIt--;
            }

            // fit line to range
            LinearFitter linearModel;
            linearModel.fit(x, y);

            // check if unexpected jump occures in next step
            double delta_y = (it+1).value()[channel] - linearModel.model((it+1).key() - x_0);

            // not in range and jump detected:
            if (!inRange && std::abs(delta_y) > jumpFactor * linearModel.getStdDev() + jumpBaseThreshold)
            {
                // offset data:
                // x_start = t_jump
                // y_offset = linear_model(t_jump)
                x_start[channel] = it.key();
                y_offset[channel] = linearModel.model(x_start[channel] - x_0);
                sigmaNoise[channel] = linearModel.getStdDev();

                reactionIsPositive = delta_y > 0;
                inRange = true;
            }
            // in range:
            else if (inRange)
            {
                // recovery start detection turned off:
                if (!detectRecoveryStart)
                {
                    x_end[channel] = selectedData.lastKey();
                    break;
                }

                // check if recovery is beginning after current point
                double recoveryThreshold = recoveryFactor * linearModel.getStdDev();
                if (reactionIsPositive ? linearModel.getM() < -recoveryThreshold : linearModel.getM() > recoveryThreshold)
                {
                    x_end[channel] = it.key();
                    break;
                }
            }
            it++;
        }

        // collect data
        it = selectedData.find(x_start[channel]);
        endIt = selectedData.constFind(x_end[channel]);

        while(it.key() <= endIt.key() && it != selectedData.constEnd())
        {
            double x = it.key() - x_start[channel];
            double y = it.value()[channel] - y_offset[channel];
            dataRange[channel].push_back(std::pair<double, double>(x, y));
            it++;
        }
    }

    emit rangeDeterminationFinished();
}

void FitWorker::fit()
{
    emit started();

    // init fitter
    LeastSquaresFitter *fitter, *fitter_lm;
    switch (type) {
    case LeastSquaresFitter::Type::SUPERPOS:
        fitter = new ADG_superpos_Fitter();
        fitter_lm = new ADG_superpos_Fitter();
        break;
    default:
        throw std::runtime_error("Unknown fitter type!");
    }

    // init parameterMap
    for (QString parameterName : fitter->getParameterNames())
        parameterMap[parameterName] = std::vector<double>(64, 0.);

    for (int channel=0; channel<MVector::nChannels; channel++)
    {
        qDebug() << "\nFit channel: " << channel;

        auto channelData = dataRange[channel];
        for (auto pair : channelData)
            qDebug() << pair.first << ", " << pair.second;

        // no jump found or unplausible range detected:
        // ignore
        if (channelData.empty() || channelData.size() < 0.15 * selectedData.size())
        {
            continue;
        }

        // fit curve to channelData
        try {
            fitter->solve(channelData, nIterations, limitFactor);
            fitter_lm->solve_lm(channelData, nIterations, limitFactor);

            auto bestFitter = fitter->residual_sum_of_sqares(channelData) < fitter_lm->residual_sum_of_sqares(channelData) ? fitter : fitter_lm;
            QList<QString> parameterNames = bestFitter->getParameterNames();
            auto params = bestFitter->getParams();

            for (int i=0; i<parameterNames.size(); i++)
            {
                QString parameterName = parameterNames[i];
                parameterMap[parameterName][channel] = params[i];
            }
            sigmaError[channel] = std::sqrt(bestFitter->residual_sum_of_sqares(channelData) / channelData.size());
            tau90[channel] = bestFitter->tau_90();
            f_t90[channel] = bestFitter->f_t_90();
            nSamples[channel] = channelData.size();
        } catch (dlib::error exception) {
            error("Error in channel " + QString::number(channel) + ": " + QString(exception.what()));
        }

        emit progressChanged(channel);
    }

    emit finished();
    emit dataSet(getHeader(), getData());
    delete fitter;
    delete  fitter_lm;
}

void FitWorker::save(QString filePath) const
{
    QFile file (filePath);

    if (!file.open(QIODevice::WriteOnly))
        throw std::runtime_error("Unable to open file:" + file.errorString().toStdString());

    QTextStream out(&file);

    // write header
    out << getHeader().join(";") << "\n";

    auto data = getData();
    for (int i=0; i<data.size(); i++)
    {
        QStringList valueStrings;
        for (double value : data[i])
            valueStrings << QString::number(value);

        out << valueStrings.join(";") << "\n";
    }
}


std::vector<double> FitWorker::getNSamples() const
{
    return nSamples;
}

bool FitWorker::rangeIsSet()
{
    for (int i=0; i<MVector::nChannels; i++)
        if (!dataRange[i].empty())
            return true;
    return false;
}

void FitWorker::setJumpBaseThreshold(double value)
{
    if (!qFuzzyCompare(value, jumpBaseThreshold))
    {
        jumpBaseThreshold = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }
}

void FitWorker::setJumpFactor(double value)
{
    if (!qFuzzyCompare(value, jumpFactor))
    {
        jumpFactor = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }

}

void FitWorker::setRecoveryFactor(double value)
{
    if (!qFuzzyCompare(value, recoveryFactor))
    {
        recoveryFactor = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }
}

void FitWorker::emitChannelRange(int channel)
{
    if (dataRange.empty())
        return;

    QList<uint> range;
    for (auto pair : dataRange[channel])
        range.append(pair.first + x_start[channel]);

    emit channelRangeProvided(channel, range);
}

void FitWorker::addToRange(int channel, QList<int> range)
{
    auto selectionKeys = selectedData.keys();
    auto *channelData = &dataRange[channel];

    for (int index : range)
    {
        int timestamp = selectionKeys[index];
        int time = timestamp  - x_start[channel];

        bool containsTime = false;

        // dont add if timestamp already part of range
        for (auto it=channelData->begin(); it!=channelData->end(); it++)
        {
            if (it->first == time)
            {
                containsTime = true;
                break;
            }
        }

        if (!containsTime)
        {
            channelData->push_back(std::pair<double, double>(time, selectedData[timestamp][channel] - y_offset[channel]));
            emit rangeRedeterminationPossible();
        }
    }

    emit rangeDeterminationFinished();
}

void FitWorker::removeFromRange(int channel, QList<int> range)
{
    auto selectionKeys = selectedData.keys();
    auto *channelData = &dataRange[channel];

    for (int index : range)
    {
        int timestamp = selectionKeys[index];
        int time = timestamp - x_start[channel];

        // erase if timestamp part of range
        for (auto it=channelData->begin(); it!=channelData->end(); it++)
        {
            if (it->first == time)
            {
                channelData->erase(it);
                emit rangeRedeterminationPossible();
                break;
            }
        }
    }

    emit rangeDeterminationFinished();
}

void FitWorker::setDetectExpositionStart(bool value)
{
    if (value != detectExpositionStart)
    {
        detectExpositionStart = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }
}

void FitWorker::setDetectRecoveryStart(bool value)
{
    if (value != detectRecoveryStart)
    {
        detectRecoveryStart = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }
}

void FitWorker::setNIterations(const int &value)
{
    if (value != nIterations)
    {
        nIterations = value;
    }
}

void FitWorker::setLimitFactor(const double &value)
{
    if (value != limitFactor)
    {
        limitFactor = value;
    }
}

QStringList FitWorker::getHeader() const
{
    // get data from the worker and emit
    QStringList header;

    header << "n samples";
    header << "sigma error";
    header << "sigma noise";
    header << QString::fromUtf8("tau90");
    header << QString::fromUtf8("f(t90)");

    for (QString parameterName : parameterMap.keys())
        header << parameterName;

    return header;
}

QList<QList<double>> FitWorker::getData() const
{
    QList<QList<double>> resultData;
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(getNSamples()));
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(getSigmaError()));
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(sigmaNoise));
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(getTau90()));
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(getF_tau90()));

    for (QString parameterName : parameterMap.keys())
        resultData << QList<double>::fromVector(QVector<double>::fromStdVector(parameterMap[parameterName]));

    return resultData;
}

std::vector<double> FitWorker::getF_tau90() const
{
    return f_t90;
}

void FitWorker::setFitBuffer(const uint &value)
{
    if (value != fitBuffer)
    {
        fitBuffer = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }
}

void FitWorker::setType(const LeastSquaresFitter::Type &value)
{
    if (value != type)
    {
        type = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }
}

std::vector<double> FitWorker::getSigmaError() const
{
    return sigmaError;
}

std::vector<double> FitWorker::getTau90() const
{
    return tau90;
}

QMap<QString, std::vector<double>> FitWorker::getParameterMap() const
{
    return parameterMap;
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

    for(QWizard::WizardButton which: {QWizard::BackButton, QWizard::NextButton, QWizard::CancelButton, QWizard::FinishButton})
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
        selectedData[timestamp] = selectedData[timestamp].getRelativeVector(mData->getBaseLevel(timestamp));
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

void ResultPage::setData(QStringList header, QList<QList<double>> data)
{
    resultTable->setRowCount(MVector::nChannels);
    resultTable->setColumnCount(data.size());

    // set header
    resultTable->setHorizontalHeaderLabels(header);

    // set values
    auto sensorFailures = mData->getSensorFailures();
    for (int column=0; column<data.size(); column++)
    {
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

int ResultPage::getCurrentChannel()
{
    return resultTable->currentRow();
}
