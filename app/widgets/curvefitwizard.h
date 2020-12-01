#ifndef CURVEFITWIZARD_H
#define CURVEFITWIZARD_H

#include <QtWidgets>
#include <QtCore>
#include <QWizard>

#include "../classes/mvector.h"
#include "../classes/leastsquaresfitter.h"
#include "../classes/measurementdata.h"

#include "../lib/spoiler.h"

#define CVWIZ_DEFAULT_MODEL_TYPE LeastSquaresFitter::Type::SUPERPOS
#define CVWIZ_DEFAULT_JUMP_BASE_THRESHOLD 0.2
#define CVWIZ_DEFAULT_JUMP_FACTOR 2.0
#define CVWIZ_DEFAULT_RECOVERY_FACTOR 0.25
#define CVWIZ_DEFAULT_BUFFER_SIZE 15
#define CVWIZ_DEFAULT_DETECT_EXPOSITION_START true
#define CVWIZ_DEFAULT_DETECT_RECOVERY_START false

class IntroPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit IntroPage(QWidget *parent = nullptr);
    ~IntroPage() override;

//    void initializePage() override;
    bool validatePage() override;

    void setRangeRedeterminationPossible();

Q_SIGNALS:
    void typeChanged(QString typeString);
    void nIterationsChanged(const int &value);
    void limitFactorChanged(const double &value);
    void jumpBaseThresholdChanged(double jumpBaseThreshold);
    void jumpFactorChanged(double jumpFactor);
    void recoveryFactorChanged (double recoveryFactor);
    void fitBufferChanged(int fitBuffer);
    void rangeDeterminationRequested() const;
    void detectExpositionChanged(bool useJump);
    void detectRecoveryChanged(bool useRecovery);

public Q_SLOTS:
    void setDetectExpositionStart(bool value);
    void setDetectRecoveryStart(bool value);

private:
    QFormLayout *detectionLayout;
    QComboBox *typeSelector;
    QCheckBox *detectExpositionStartCheckBox, *detectRecoveryCheckBox;
    QDoubleSpinBox *limitFactorSpinBox, *jumpFactorSpinBox, *jumpBaseThresholdSpinBox, *recoveryFactorSpinBox;
    QSpinBox *nIterationsSpinBox, *fitBufferSpinBox;
    bool rangeRedeterminationPossible = false;
};


class FitWorker: public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit FitWorker(MeasurementData* mData, QObject *parent = nullptr);

    std::vector<double> getTau90() const;

    std::vector<double> getSigmaError() const;

    std::vector<double> getNSamples() const;

    bool rangeIsSet();

    std::vector<double> getF_tau90() const;

    void run() override;

public Q_SLOTS:
    void init();
    void fitChannel(size_t channel);
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
    QStringList getTooltips() const;
    QList<QList<double>> getData() const;

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
    std::vector<double> sigmaError, tau90, f_t90, sigmaNoise;
    std::vector<double> nSamples;
    MeasurementData* mData;
    QMap<uint, AbsoluteMVector> selectedData;
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
    int nIterations = LEAST_SQUARES_N_ITERATIONS;
    double limitFactor = LEAST_SQUARES_LIMIT_FACTOR;
};

class FitPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit FitPage(QWidget *parent = nullptr);
    ~FitPage() override;

    bool isComplete() const override;

Q_SIGNALS:
    void fitRequested();

public Q_SLOTS:
    void startFit();
    void onStarted();
    void onFinished();
    void onProgressChanged(int value);
    void onError(QString errorMessage);
    void onRangeDeterminationStarted();
    void onRangeDeterminationFinished();

private:
    QProgressBar *progressbar;
    QPushButton *startButton;
};

class ResultPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ResultPage(MeasurementData *mData, QWidget *parent = nullptr);
    ~ResultPage();

    bool isComplete() const;

    int getCurrentChannel();

    void setDataSaved(bool value);

Q_SIGNALS:
    void saveResultsRequested();
    void channelRangeRequested(int channel);
    void removeFromRange(int channel, QList<int> range);
    void addToRange(int channel, QList<int> range);

public Q_SLOTS:
    void setData(QStringList header, QStringList tooltips, QList<QList<double>> data);
    void setChannelRange(int channel, QList<uint> range);
    void resultSelectionChanged();
    void channelDataSelectionChanged();
    void requestRangeRemoval();
    void requestRangeExtension();

private:
    QTableWidget *resultTable, *rangeTable;
    QGroupBox *resultBox, *channelDataBox;
    QPushButton *saveButton, *minusButton, *addButton;
    MeasurementData *mData;
    QMap<uint, AbsoluteMVector> selectedData;

    bool dataSaved = false;
};

class CurveFitWizard : public QWizard
{
    Q_OBJECT

public:
    enum { Page_Intro, Page_Fit, Page_Result};

    explicit CurveFitWizard(MeasurementData* mData, QWidget* parent = nullptr);
    ~CurveFitWizard();

public Q_SLOTS:
    void selectType();
    void saveData();
    void updateChannelRange();
    void fitCurves();

private:
    IntroPage* introPage;
    FitPage* fitPage;
    ResultPage* resultPage;
    FitWorker* worker;
    size_t nChannels;
};

#endif // CURVEFITWIZARD_H
