#include "curvefitworker.h"

CurveFitWorker::CurveFitWorker(MeasurementData* mData, QObject *parent):
    QObject(parent),
    QRunnable(),
    sigmaError(MVector::nChannels, 0.),
    tau90(MVector::nChannels, 0.),
    f_t90(MVector::nChannels, 0.),
    sigmaNoise(MVector::nChannels, 0.),
    t10_recovery(MVector::nChannels, 0.),
    nSamples(MVector::nChannels, 0),
    mData(mData),
    dataRange(MVector::nChannels, std::vector<std::pair<double, double>>()),
    y_offset(MVector::nChannels, 0),
    x_start(MVector::nChannels, mData->getFitMap().firstKey()),
    fitValid(MVector::nChannels, true),
    relativeData(mData->getRelativeData())
{
    fitData = mData->getFitMap();
    auto relativeData = mData->getRelativeData();

    Q_ASSERT(!relativeData.isEmpty());
    Q_ASSERT(!fitData.isEmpty());

    // convert selectedData to relative
    for (uint timestamp : fitData.keys())
        fitData[timestamp] = relativeData[timestamp];
}

void CurveFitWorker::run()
{
    Q_ASSERT(ch < mData->nChannels());

    if (ch == 0)
        emit started();

    // select channel to be fitted
    mutex.lock();
    size_t channel = ch;
    ch++;
    mutex.unlock();

    // fit channel
    fitChannel(channel);

    // signal progress
    mutex.lock();
    channelsFinished++;
    emit progressChanged(channelsFinished);
    mutex.unlock();

    if (channelsFinished == mData->nChannels())
    {
        QStringList header = getTableHeader();
        QStringList tooltips = getTooltips();
        auto data = getData();

        emit finished();
        emit dataSet(header, tooltips, data);
    }
}

void CurveFitWorker::init()
{
    // reset ch
    ch = 0;
    channelsFinished = 0;

    // reset parameters
    std::unique_ptr<LeastSquaresFitter> fitter;
    switch (type) {
    case LeastSquaresFitter::Type::SUPERPOS:
        fitter.reset(new ADG_superpos_Fitter());
        break;
    default:
        throw std::runtime_error("Unknown fitter type!");
    }

    parameterNames = fitter->getParameterNames();
    fitTooltips = fitter->getTooltips();

    parameterData.clear();
    for ( int i=0; i<fitTooltips.size(); i++ )
        parameterData << std::vector<double>(MVector::nChannels, 0.);

    sigmaError = std::vector<double>(sigmaError.size(), 0.);
    tau90 = std::vector<double>(tau90.size(), 0.);
    f_t90 = std::vector<double>(f_t90.size(), 0.);
    sigmaNoise =  std::vector<double>(sigmaNoise.size(), 0.);
    t10_recovery =  std::vector<double>(t10_recovery.size(), 0.);
    nSamples =  std::vector<double>(nSamples.size(), 0.);
    fitValid = std::vector<bool>(fitValid.size(), true);

    // determine channel ranges
    determineChannelRanges();
}

void CurveFitWorker::setChannelRanges(uint start, uint end)
{
    // collect fitData from key range [start; end]
    fitData.clear();
    for (uint timestamp : relativeData.keys())
        if (timestamp >= start && timestamp <= end)
            fitData[timestamp] = relativeData[timestamp];

    for (uint channel=0; channel<MVector::nChannels; channel++)
    {

    }

    // execute channel range determination without detection of exposition & recovery
    bool prevDetExpSt = detectExpositionStart;
    bool prevDetRecSt = detectRecoveryStart;

    setDetectExpositionStart(false);
    setDetectRecoveryStart(false);

    init();

    setDetectExpositionStart(prevDetExpSt);
    setDetectRecoveryStart(prevDetRecSt);
}

void CurveFitWorker::determineChannelRanges()
{
    emit rangeDeterminationStarted();

    for (int i=0; i<MVector::nChannels; i++)
        dataRange[i].clear();

    auto sensorFailures = mData->getSensorFailures();

    std::vector<uint> x_end(MVector::nChannels, fitData.lastKey());
    for (size_t channel=0; channel<MVector::nChannels; channel++)
    {
        // ignore channels with sensor failure flags
        if (sensorFailures[channel])
            continue;

        // adjust data range for channel
        auto it = relativeData.find(fitData.firstKey());
        auto endIt = relativeData.constFind(fitData.lastKey());

        bool inRange = false;
        bool reactionIsPositive = true;

        // exposition start detection turned off:
        if (!detectExpositionStart)
        {
            x_start[channel] = fitData.firstKey();
            y_offset[channel] = fitData.first()[channel];
            inRange = true;

            // calculate drift noise
            double x0 = fitData.firstKey();
            double y0 = fitData.first()[channel];
            std::vector<double> x, y;
            for (int t=x0-1; t>x0-fitBuffer-1; t--)
            {
                if (relativeData.contains(t))
                {
                    x.push_back(t - x0);
                    y.push_back(relativeData[t][channel] - y0);
                }
            }

            if (x.size() > 3)
            {
                // fit line to range
                LinearFitter linearModel;
                linearModel.fit(x, y);

                sigmaNoise[channel] = linearModel.getStdDev();
            }
        }

        // collect points for linear fit:
        // before exposition start:
        // -> *fitBuffer* seconds before current point
        // after exposition start:
        // -> *fitBuffer* seconds after current point
        uint x_0 = it.key();
        while(it.key() <= endIt.key() && it != relativeData.constEnd())
        {
            // collect vectors in range [innerIt.key(); innerIt.key() + CURVE_FIT_CHANNEL_BUFFER]
            std::vector<double> x, y;
            auto lineIt = it;

            while(std::abs(static_cast<long>(lineIt.key()) - static_cast<long>(it.key())) < fitBuffer)
            {
                x.push_back(lineIt.key() - x_0);
                y.push_back(lineIt.value()[channel]);

                // in range:
                // fit line to subsequent values
                if (inRange && lineIt != relativeData.constEnd()-1)
                    lineIt++;
                // not in range:
                // fit line to previous values
                else if (!inRange && lineIt != relativeData.constBegin())
                    lineIt--;
                else
                    break;
            }

            // fit line to range
            LinearFitter linearModel;
            linearModel.fit(x, y);

            // check if unexpected jump occures in next step
            double delta_y;
            if (x.size() > 3 && it+1 != relativeData.constEnd())   // drift fitted with at least 3 values
                delta_y = (it+1).value()[channel] - linearModel.model((it+1).key() - x_0);
            else
                delta_y = 0;

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
                    x_end[channel] = fitData.lastKey();
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
        auto collectionIt = fitData.find(x_start[channel]);
        auto collectionEndIt = fitData.constFind(x_end[channel]);

        while(collectionIt.key() <= collectionEndIt.key() && collectionIt != fitData.constEnd())
        {
            double x = collectionIt.key() - x_start[channel];
            double y = collectionIt.value()[channel] - y_offset[channel];
            dataRange[channel].push_back(std::pair<double, double>(x, y));
            collectionIt++;
        }
    }

    emit rangeDeterminationFinished();
}

/*!
 * \brief CurveFitWorker::fitChannel fits LeastSquaresFitter to \param channel.
 * Two fitting algorithms are used multiple times. The best valid result is used and its parameters and metrics are stored.
 * If there is no valid result, fitValid is set to false. Channels with failures are ignored and fitValid also is set to false.
 * \param channel
 */
void CurveFitWorker::fitChannel(size_t channel)
{
    // failing channel: ignore
    if (mData->getSensorFailures()[channel])
    {
        fitValid[channel] = false;
        qDebug() << "Skipping channel " << channel+1 << " (channel failure)";

        return;
    }

    qDebug() << "Fitting channel " << channel+1;

    // init fitter
    std::shared_ptr<LeastSquaresFitter> fitter, fitter_lm;
    switch (type) {
    case LeastSquaresFitter::Type::SUPERPOS:
        fitter.reset(new ADG_superpos_Fitter());
        fitter_lm.reset(new ADG_superpos_Fitter());
        break;
    default:
        throw std::runtime_error("Unknown fitter type!");
    }

    auto channelData = dataRange[channel];
//    for (auto pair : channelData)
//        qDebug() << pair.first << ", " << pair.second;

    // no jump found or detected range too small:
    // ignore
    if (!channelData.empty() || !(channelData.size() < 0.15 * fitData.size()))
    {

        try {
            // fit curve to channelData:
            // usage of two algorithms (solve vs solvs_lm)
            // results are compared afterwards
            fitter->solve(channelData, nIterations, limitFactor);
            double solve_error = fitter->residual_sum_of_sqares(channelData);

            fitter_lm->solve_lm(channelData, nIterations, limitFactor);
            double solve_lm_error = fitter->residual_sum_of_sqares(channelData);

            // validate parameters:
            // invalid results should be ignored in the fitting process,
            // however edge cases may produce invalid parameters
            double lastVal = channelData.back().second;
            bool solve_valid = fitter->parameters_valid(limitFactor * lastVal);
            bool solve_lm_valid = fitter->parameters_valid(limitFactor * lastVal);

            std::shared_ptr<LeastSquaresFitter> bestFitter;
            double bestError = qInf();
            if ( solve_valid && solve_lm_valid ) {   // parameters of both fitters valid
                bestFitter = solve_error < solve_lm_error ? fitter : fitter_lm;
                bestError = solve_error < solve_lm_error ? solve_error : solve_lm_error;
            } else if ( solve_valid ) { // only fitter params valid
                bestFitter = fitter;
                bestError = solve_error;
            } else if ( solve_lm_valid ) {  // only fitter_lm params valid
                bestFitter = fitter_lm;
                bestError = solve_lm_error;
            } else {    // invalid results -> return
                fitValid[channel] = false;
                return;
            }

            auto params = bestFitter->getParams();

            for (size_t i=0; i<params.size(); i++)
            {
                parameterData[i][channel] = params[i];
            }
            sigmaError[channel] = std::sqrt(bestError / channelData.size());
            tau90[channel] = bestFitter->tau_90();
            f_t90[channel] = bestFitter->f_t_90();
            nSamples[channel] = channelData.size();

            // after curve fit:
            // recovery time
            determineTRecovery(channel);
        } catch (dlib::error exception) {
            error("Error in channel " + QString::number(channel) + ": " + QString(exception.what()));
        }
    }
}

/*!
 * \brief CurveFitWorker::determineTRecovery determines time until channel recovers to 10% of the plateau value.
 * Rolling average values are used to make the determination more robust.
 * The result is stored in t10_recovery[channel].
 * if this time is greater than t_recovery or the measurement stops before, t_recovery is used
 *
 * \param channel
 * \param tAverage delta time within which values are averaged to determine recovery
 */
void CurveFitWorker::determineTRecovery(size_t channel, int tAverage)
{
    uint recovery_start = fitData.lastKey();
    double recovery_threshold = f_t90[channel] / 9;

    auto it = relativeData.find(recovery_start);
    while (it != relativeData.constEnd() && it.key()-recovery_start < t_recovery)
    {
        // collect vectors within tAverage of it
        QList<double> rollingAverageValues;
        for (uint t=it.key()-tAverage; t<it.key()+tAverage; t++)
        {
            if (relativeData.contains(t))
                rollingAverageValues <<  relativeData[t][channel];
        }

        // calculate rolling average
        double rollingAverage = 0.;
        for (auto value : rollingAverageValues)
            rollingAverage += value / rollingAverageValues.size();

        // detect recovery
        if (rollingAverage < recovery_threshold)
        {
            t10_recovery[channel] = it.key() - recovery_start;
            break;
        }

        it++;
    }

    // check if recovery time was set
    if (qFuzzyIsNull(t10_recovery[channel]))
        t10_recovery[channel] = t_recovery;
}

void CurveFitWorker::save(QString filePath) const
{
    QFile file (filePath);

    if (!file.open(QIODevice::WriteOnly))
        throw std::runtime_error("Unable to open file:" + file.errorString().toStdString());

    QTextStream out(&file);

    // header
    QStringList topHeader = getHeader();
    out << ";" << topHeader.join(";") + "\n";

    // data
    auto data = getData();

    QList<QStringList> values;
    for (int channel=0; channel<mData->nChannels(); channel++)
    {
        QStringList values;
        values << "ch" + QString::number(channel+1);

        for (int i=0; i<data.size(); i++)
            values << QString::number(data[i][channel]);

        out << values.join(";") << "\n";
    }
}


std::vector<double> CurveFitWorker::getNSamples() const
{
    return nSamples;
}

bool CurveFitWorker::rangeIsSet()
{
    for (int i=0; i<MVector::nChannels; i++)
        if (!dataRange[i].empty())
            return true;
    return false;
}

void CurveFitWorker::setJumpBaseThreshold(double value)
{
    if (!qFuzzyCompare(value, jumpBaseThreshold))
    {
        jumpBaseThreshold = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }
}

void CurveFitWorker::setJumpFactor(double value)
{
    if (!qFuzzyCompare(value, jumpFactor))
    {
        jumpFactor = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }

}

void CurveFitWorker::setRecoveryFactor(double value)
{
    if (!qFuzzyCompare(value, recoveryFactor))
    {
        recoveryFactor = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }
}

void CurveFitWorker::emitChannelRange(int channel)
{
    if (dataRange.empty())
        return;

    QList<uint> range;
    for (auto pair : dataRange[channel])
        range.append(pair.first + x_start[channel]);

    emit channelRangeProvided(channel, range);
}

void CurveFitWorker::addToRange(int channel, QList<int> range)
{
    auto selectionKeys = fitData.keys();
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
            channelData->push_back(std::pair<double, double>(time, fitData[timestamp][channel] - y_offset[channel]));
            emit rangeRedeterminationPossible();
        }
    }

    emit rangeDeterminationFinished();
}

void CurveFitWorker::removeFromRange(int channel, QList<int> range)
{
    auto selectionKeys = fitData.keys();
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

void CurveFitWorker::setDetectExpositionStart(bool value)
{
    if (value != detectExpositionStart)
    {
        detectExpositionStart = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }
}

void CurveFitWorker::setDetectRecoveryStart(bool value)
{
    if (value != detectRecoveryStart)
    {
        detectRecoveryStart = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }
}

void CurveFitWorker::setNIterations(const int &value)
{
    if (value != nIterations)
    {
        nIterations = value;
    }
}

void CurveFitWorker::setLimitFactor(const double &value)
{
    if (value != limitFactor)
    {
        limitFactor = value;
    }
}

QStringList CurveFitWorker::getTableHeader() const
{
    // get data from the worker and emit
    QStringList header;

    // channel state
    header << "channel\nfailure";
    header << "fit valid";

    // metrics
    header << "number of\nsamples";
    header << "sigma error\n[ % ]";
    header << "sigma noise\n[ % ]";
    header << QString::fromUtf8("tau90\n[ s ]");
    header << QString::fromUtf8("f(t90)\n[ % ]");
    header << "t_recovery\n[ s ]";

    // curve parameters
    header << parameterNames;

    return header;
}

QStringList CurveFitWorker::getHeader() const
{
    QString header = getTableHeader().join(";");
    header = header.replace('\n', ' ');

    return header.split(";");
}

QStringList CurveFitWorker::getTooltips() const
{
    QStringList tooltips;
    // channel state
    tooltips << "channel failure:\nno curve is fitted to failed channels";
    tooltips << "fit valid\nsignals wether the curve fitted for the channel is valid";

    // metrics
    tooltips << "number of samples:\nnumber of data points used for fitting the curve";                                 // number of samples
    tooltips << "sigma error:\nstandard deviation of the exposition data in relation to the fitted curve";              // sigma error
    tooltips << "sigma noise:\nstandard deviation before the start of the exposition in relation to the linear drift";  // sigma noise                                                // sigma noise
    tooltips << "tau90:\ntime from start of the exposition until 90% of the plateau is reached";                       // tau90
    tooltips << "f(t90):\n90% of the plateau height";                                                                   // f(t90)
    tooltips << "t_recovery: time in s from end of exposition until 10% of the plateau is reached";

    // curve paramters
    tooltips.append(fitTooltips);

    return  tooltips;
}

QList<QList<double>> CurveFitWorker::getData() const
{
    QList<QList<double>> resultData;

    // channel state
    auto sensorFailures = mData->getSensorFailures();
    QList<double> sensorFailureList;
    QList<double> fitValidList;
    for (uint i=0; i<fitValid.size(); i++) {
        sensorFailureList << static_cast<double>(sensorFailures[i]);
        fitValidList << static_cast<double>(fitValid[i]);
    }
    resultData << sensorFailureList;
    resultData << fitValidList;

    // metrics
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(getNSamples()));

    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(getSigmaError()));
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(sigmaNoise));
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(getTau90()));
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(getF_tau90()));
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(t10_recovery));

    // curve parameters
    for (size_t i=0; i<parameterData.size(); i++)
        resultData << QList<double>::fromVector(QVector<double>::fromStdVector(parameterData[i]));

    return resultData;
}

MeasurementData* CurveFitWorker::getMData() const
{
    return mData;
}

void CurveFitWorker::setT_recovery(int value)
{
    // convert into minutes
    if (value > 0)
        value = 60 * value;
    t_recovery = value;
}

std::vector<double> CurveFitWorker::getF_tau90() const
{
    return f_t90;
}

void CurveFitWorker::setFitBuffer(const uint &value)
{
    if (value != fitBuffer)
    {
        fitBuffer = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }
}

void CurveFitWorker::setType(const LeastSquaresFitter::Type &value)
{
    if (value != type)
    {
        type = value;
        if (rangeIsSet())
            emit rangeRedeterminationPossible();
    }
}

std::vector<double> CurveFitWorker::getSigmaError() const
{
    return sigmaError;
}

std::vector<double> CurveFitWorker::getTau90() const
{
    return tau90;
}

/*!
 * \brief AutomatedFitWorker::AutomatedFitWorker automatically
 * \param mData
 * \param timeout time until curve fit is canceled by timeout
 * \param nCores number of threads used simultaneously
 * \param t_exposition time of exposition
 * exposition starts at the start of the measurement + t_offset
 * Default (t_exposition=-1): exposition is assumed to go until the end of the measurement.
 * \param t_offset time offset until the start of the exposition
 * \param parent
 */
AutomatedFitWorker::AutomatedFitWorker(MeasurementData *mData, int timeout, int nCores, int t_exposition, int t_recovery, int t_offset, QObject *parent):
    QObject(parent),
    mData(mData),
    timeoutInS(timeout),
    t_exposition(t_exposition),
    t_offset(t_offset)
{
    auto absoluteData = mData->getAbsoluteData();
    if (absoluteData.isEmpty())
        throw std::runtime_error("Measurement loaded is empty!");

    worker = new CurveFitWorker(mData, this);

    // adjust default parameters:
    // timeout: 10s / channel
    if (timeoutInS < 0)
        timeoutInS = mData->nChannels() * 10;
    // nCores: all available
    if (nCores < 0)
        nCores = QThread::idealThreadCount();

    t_exposition_start = absoluteData.begin().key() + t_offset;
    t_exposition_end = t_exposition>=0 ? t_exposition_start + t_exposition : absoluteData.end().key();
    this->t_recovery = t_recovery>=0 ? t_recovery : absoluteData.end().key() - t_exposition_end;
}

AutomatedFitWorker::~AutomatedFitWorker()
{
}

void AutomatedFitWorker::fit()
{
    worker->setT_recovery(t_recovery);
    worker->setChannelRanges(t_exposition_start, t_exposition_end);

    // prepare event loop:
    // wait for finished signal of the curve fit
    // time
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect( worker, &CurveFitWorker::finished, &loop, &QEventLoop::quit );
    connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit );

    //  execute channel fits in the thread pool
    worker->setAutoDelete(false);
    QThreadPool::globalInstance()->setMaxThreadCount(nCores);
    qDebug() << "\n--------\nStarting curve fit:";
    qDebug() << "max thread count:\t" << QString::number(QThreadPool::globalInstance()->maxThreadCount());
    qDebug() << "t_offset:\t" << QString::number(t_offset);
    qDebug() << "t_exposition:\t" << QString::number(t_exposition);
    qDebug() << "t_recovery:\t" << QString::number(t_recovery);
    for (size_t i=0; i<mData->nChannels(); i++)
        QThreadPool::globalInstance()->start(worker);

    // start event loop & timeout timer
    timer.start(timeoutInS*1000);
    loop.exec();

    if(timer.isActive())
        qDebug("Curve fit terminated successfully");
    else
        qDebug("Error: Curve fit terminated due to timeout");
}

void AutomatedFitWorker::save(QString fileName)
{
    worker->save(fileName);
}
