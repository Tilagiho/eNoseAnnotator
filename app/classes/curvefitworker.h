#ifndef CURVEFITWORKER_H
#define CURVEFITWORKER_H

#include <QObject>
#include <QtCore>

#include "measurementdata.h"
#include "defaultSettings.h"

class CurveFitWorker: public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit CurveFitWorker(MeasurementData* mData, QObject *parent = nullptr);

    std::vector<double> getTau90() const;

    std::vector<double> getSigmaError() const;

    std::vector<double> getNSamples() const;

    bool rangeIsSet();

    std::vector<double> getF_tau90() const;

    void run() override;

    void setT_recovery(int value);

public Q_SLOTS:
    void init();
    void fitChannel(size_t channel);
    void determineTRecovery(size_t channel, int tAverage=4);
    void setChannelRanges(uint start, uint end);
    void determineChannelRanges();
    void save(QString filePath) const;

    void setType(const LeastSquaresFitter::Type &value);
    void setFitBuffer(const uint &value);
    void setJumpBaseThreshold(double value);
    void setJumpFactor(double value);
    void setRecoveryFactor(double value);

    void emitChannelRange(int channel);

    void removeFromRange(int channel, QList<int> range);
    void addToRange(int channel, QList<int> range);

    void setDetectExpositionStart(bool value);
    void setDetectRecoveryStart(bool value);

    void setNIterations(const int &value);
    void setLimitFactor(const double &value);

    QStringList getHeader() const;
    QStringList getTableHeader() const;
    QStringList getTooltips() const;
    QList<QList<double>> getData() const;

    MeasurementData* getMData() const;

Q_SIGNALS:
    void started();
    void progressChanged(int value);
    void finished();
    void dataSet(QStringList header, QStringList tooltips, QList<QList<double>> data);
    void error(QString errorMessage);
    void rangeRedeterminationPossible();
    void rangeDeterminationStarted();
    void rangeDeterminationFinished();
    void channelRangeProvided(int channel, QList<uint> channelRange);

private:
    size_t ch = 0;
    int channelsFinished = 0;
    QMutex mutex;

    QStringList fitTooltips;
    QList<QString> parameterNames;
    QList<std::vector<double>> parameterData;
    std::vector<double> sigmaError, tau90, f_t90, sigmaNoise, t10_recovery;
    std::vector<double> nSamples;
    std::vector<bool> fitValid;
    MeasurementData* mData;
    QMap<uint, AbsoluteMVector> fitData;
    QMap<uint, RelativeMVector> relativeData;
//    std::vector<uint> x_start, x_end;
    std::vector<std::vector<std::pair<double, double>>> dataRange;

    std::vector<double> y_offset;
    std::vector<uint> x_start;
    LeastSquaresFitter::Type type = CVWIZ_DEFAULT_MODEL_TYPE;
    uint fitBuffer = CVWIZ_DEFAULT_BUFFER_SIZE;
    double jumpFactor = CVWIZ_DEFAULT_JUMP_FACTOR;
    double recoveryFactor = CVWIZ_DEFAULT_RECOVERY_FACTOR;
    double jumpBaseThreshold = CVWIZ_DEFAULT_JUMP_BASE_THRESHOLD;
    bool detectExpositionStart = CVWIZ_DEFAULT_DETECT_EXPOSITION_START;
    bool detectRecoveryStart = CVWIZ_DEFAULT_DETECT_RECOVERY_START;
    int nIterations = LEAST_SQUARES_N_FITS;
    double limitFactor = LEAST_SQUARES_LIMIT_FACTOR;

    int t_recovery = 60*CVWIZ_DEFAULT_RECOVERY_TIME;
};

class AutomatedFitWorker: public QObject
{
    Q_OBJECT

public:
    explicit AutomatedFitWorker(MeasurementData *mData, int timeout=-1, int nCores=-1, int t_exposition=-1, int t_recovery=-1, int t_offset=0, QObject *parent = nullptr);
    ~AutomatedFitWorker();

public slots:
    void fit();
    void save(QString fileName);

protected:
    MeasurementData* mData;
    CurveFitWorker* worker;
    int timeoutInS;
    int nCores;
    uint t_exposition_start;
    uint t_exposition_end;
    uint t_recovery;
};

#endif // CURVEFITWORKER_H
