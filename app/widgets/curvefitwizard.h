#ifndef CURVEFITWIZARD_H
#define CURVEFITWIZARD_H

#include <QtWidgets>
#include <QtCore>
#include <QWizard>

#include "../classes/curvefitworker.h"
#include "../classes/mvector.h"
#include "../classes/leastsquaresfitter.h"
#include "../classes/measurementdata.h"

#include "../lib/spoiler.h"

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
    void recoveryTimeChanged(int recoveryTime);
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
    QSpinBox *nIterationsSpinBox, *fitBufferSpinBox, *recoveryTimeSpinBox;
    bool rangeRedeterminationPossible = false;
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
    CurveFitWorker* worker;
    size_t nChannels;
};

#endif // CURVEFITWIZARD_H
