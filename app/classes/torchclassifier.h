#ifndef TORCHCLASSIFIER_H
#define TORCHCLASSIFIER_H

#include <QObject>
#include <QtCore>

#undef slots
#include <torch/script.h>
#define slots Q_SLOTS

#include "annotation.h"

class TorchClassifier : public QObject
{
    Q_OBJECT

public:
    explicit TorchClassifier(QObject *parent= nullptr, QString filename = "", bool* loadOk = nullptr, QString *errorString = nullptr, int nInputs=8);

    Annotation getAnnotation (std::vector<double> input);

    QString getName() const;

    QString getFilename() const;


    bool getIsInputAbsolute() const;

    QStringList getClassNames() const;

    int getN() const;

    int getM() const;

    QString getPresetName() const;

signals:
    void isInputAbsoluteSet (bool);

private:
    // torch module
    torch::jit::script::Module module;

    // meta info
    bool isInputAbsolute = false;
    QStringList classNames{};
    QString name;
    QString filename;
    QString presetName = "None";
    int N, M;
    std::vector<double> mean_vector, stdev_vector;

    at::Tensor forward (std::vector<double> input, bool asChances=false);
    std::vector<double> normalise(std::vector<double> input);
};

#endif // TORCHCLASSIFIER_H
