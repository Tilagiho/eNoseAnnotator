#include "curvefitworker.h"

CurveFitWorker::CurveFitWorker(MeasurementData* mData, QObject *parent):
    QObject(parent),
    QRunnable(),
    sigmaError(MVector::nChannels, 0.),
    tau90(MVector::nChannels, 0.),
    f_t90(MVector::nChannels, 0.),
    sigmaNoise(MVector::nChannels, 0.),
    nSamples(MVector::nChannels, 0),
    mData(mData),
    dataRange(MVector::nChannels, std::vector<std::pair<double, double>>()),
    y_offset(MVector::nChannels, 0),
    x_start(MVector::nChannels, mData->getFitMap().firstKey())
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

    if (channelsFinished == mData->getAbsoluteData().first().getSize())
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

    // init parameters
    LeastSquaresFitter *fitter;
    switch (type) {
    case LeastSquaresFitter::Type::SUPERPOS:
        fitter = new ADG_superpos_Fitter();
        break;
    default:
        throw std::runtime_error("Unknown fitter type!");
    }

    parameterNames = fitter->getParameterNames();
    fitTooltips = fitter->getTooltips();

    parameterData.clear();
    for ( int i=0; i<fitTooltips.size(); i++ )
        parameterData << std::vector<double>(MVector::nChannels, 0.);

    delete fitter;
}

void CurveFitWorker::determineChannelRanges()
{
    emit rangeDeterminationStarted();

    for (int i=0; i<MVector::nChannels; i++)
        dataRange[i].clear();

    auto relativeData = mData->getRelativeData();
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

void CurveFitWorker::fitChannel(size_t channel)
{
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

    qDebug() << "\nFit channel: " << channel;

    auto channelData = dataRange[channel];
//    for (auto pair : channelData)
//        qDebug() << pair.first << ", " << pair.second;

    // no jump found or unplausible range detected:
    // ignore
    if (!channelData.empty() || !(channelData.size() < 0.15 * fitData.size()))
    {
        // fit curve to channelData
        try {
            fitter->solve(channelData, nIterations, limitFactor);
            fitter_lm->solve_lm(channelData, nIterations, limitFactor);

            auto bestFitter = fitter->residual_sum_of_sqares(channelData) < fitter_lm->residual_sum_of_sqares(channelData) ? fitter : fitter_lm;
            QList<QString> parameterNames = bestFitter->getParameterNames();
            auto params = bestFitter->getParams();

            for (size_t i=0; i<parameterNames.size(); i++)
            {
                parameterData[i][channel] = params[i];
            }
            sigmaError[channel] = std::sqrt(bestFitter->residual_sum_of_sqares(channelData) / channelData.size());
            tau90[channel] = bestFitter->tau_90();
            f_t90[channel] = bestFitter->f_t_90();
            nSamples[channel] = channelData.size();
        } catch (dlib::error exception) {
            error("Error in channel " + QString::number(channel) + ": " + QString(exception.what()));
        }
    }

    mutex.lock();
    channelsFinished++;
    emit progressChanged(channelsFinished);
    mutex.unlock();

    delete fitter;
    delete  fitter_lm;
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

    header << "number of\nsamples";
    header << "sigma error\n[ % ]";
    header << "sigma noise\n[ % ]";
    header << QString::fromUtf8("tau90\n[ s ]");
    header << QString::fromUtf8("f(t90)\n[ % ]");

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

    tooltips << "number of samples:\nnumber of data points used for fitting the curve";                                 // number of samples
    tooltips << "sigma error:\nstandard deviation of the exposition data in relation to the fitted curve";              // sigma error
    tooltips << "sigma noise:\nstandard deviation before the start of the exposition in relation to the linear drift";  // sigma noise                                                // sigma noise
    tooltips << "tau90:\ntime from start of the exposition until 90% of the plateau is reached";                       // tau90
    tooltips << "f(t90):\n90% of the plateau height";                                                                   // f(t90)

    tooltips.append(fitTooltips);

    return  tooltips;
}

QList<QList<double>> CurveFitWorker::getData() const
{
    QList<QList<double>> resultData;
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(getNSamples()));
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(getSigmaError()));
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(sigmaNoise));
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(getTau90()));
    resultData << QList<double>::fromVector(QVector<double>::fromStdVector(getF_tau90()));

    for (size_t i=0; i<parameterData.size(); i++)
        resultData << QList<double>::fromVector(QVector<double>::fromStdVector(parameterData[i]));

    return resultData;
}

MeasurementData* CurveFitWorker::getMData() const
{
    return mData;
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

AutomatedFitWorker::AutomatedFitWorker(MeasurementData *mData, int timeout, int nCores, QObject *parent):
    mData(mData),
    QObject(parent),
    timeoutInS(timeout)
{
    worker = new CurveFitWorker(mData, this);

    // adjust default parameters:
    // timeout: 10s / channel
    if (timeoutInS < 0)
        timeoutInS = mData->nChannels() * 10;
    // nCores: all available
    if (nCores < 0)
        nCores = QThread::idealThreadCount();
}

AutomatedFitWorker::~AutomatedFitWorker()
{
    delete worker;
}

void AutomatedFitWorker::fit()
{
    worker->init();

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
    for (size_t i=0; i<mData->nChannels(); i++)
        QThreadPool::globalInstance()->start(worker);

    // start event loop & timeout timer
    timer.start(timeoutInS*1000);
    loop.exec();

    if(timer.isActive())
        qDebug("success");
    else
        qDebug("timeout");
}

void AutomatedFitWorker::save(QString fileName)
{
    worker->save(fileName);
}
