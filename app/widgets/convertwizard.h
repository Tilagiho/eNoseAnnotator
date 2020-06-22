#ifndef CONVERTWIZARD_H
#define CONVERTWIZARD_H

#include <QtWidgets>
#include <QtCore>
#include <QWizard>

#include "../classes/mvector.h"

class ConvertWizard : public QWizard
{
    Q_OBJECT

public:
//    static std::vector<int> functionalisations;

    explicit ConvertWizard(QWidget* parent = nullptr);
    ~ConvertWizard();

private:
    QWizardPage* createIntroPage();
    QWizardPage* createFileSelectionPage();
};

class FileSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit FileSelectionPage(QWidget *parent = 0);

    bool validatePage() override;
    bool isComplete() const override;

private:
    QLabel *sourceFilesInfoLabel;
    QLineEdit *sourceFilesLineEdit;
    QPushButton *sourceFilesButton;

    QLabel *targetDirInfoLabel;
    QLineEdit *targetDirLineEdit;
    QPushButton *targetDirButton;

//    QLabel* funcInfoLabel;
//    QLabel* funcLabel;
//    QPushButton* funcButton;

    QLabel* sensorInfoLabel;
    QLineEdit *sensorIDLineEdit;

//    QLabel* nChannelsInfoLabel;
//    QSpinBox *nChannelsSpinBox;

    void getSourceFiles();
    void getTargetDir();
    void getFuncs();
};

class ConvertWorker: public QObject
{
    Q_OBJECT

public:
    explicit ConvertWorker(QObject *parent = nullptr): QObject(parent)
    {}

public Q_SLOTS:
    void convert(const QStringList sourceFilenames, const QString targetDir);
    void resume();
    void cancel();

Q_SIGNALS:
    void started();
    void progressChanged(int value);
    void finished();
    void error(QString errorMessage);

private:
    QMutex sync;
    QWaitCondition pauseCond;

    void convertFile(QString filename, QString targetDir);
};

class ConversionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ConversionPage(QWidget *parent = nullptr);
    ~ConversionPage() override;

    void initializePage() override;

Q_SIGNALS:
    void resumeConversion();

private Q_SLOTS:
    void startConversion();
    void onStarted();
    void onFinished();
    void onProgressChanged(int value);
    void onError(QString errorMessage);

private:
    ConvertWorker worker;
    QThread *thread;
    QLabel *label;
    QProgressBar *progressbar;

    QStringList filenames;
    QString targetDir;
//    std::vector<int> functionalisation;
    int nChannels = MVector::nChannels;
};

#endif // CONVERTWIZARD_H
