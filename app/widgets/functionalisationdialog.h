#ifndef FUNCTIONALISATIONDIALOG_H
#define FUNCTIONALISATIONDIALOG_H

#include <QtCore>
#include <QDialog>
#include <QSpinBox>
#include <QLabel>
#include <QComboBox>
#include <QDialogButtonBox>

#include "../classes/mvector.h"
#include "../classes/defaultSettings.h"
#include "../classes/functionalisation.h"

class FunctionalisationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FunctionalisationDialog(QString presetDir, ulong nChannels=MVector::nChannels, QWidget *parent = nullptr);
    ~FunctionalisationDialog();

    void setFunctionalisation(const Functionalisation &functionalisation);

    Functionalisation getFunctionalisation();

    QString presetName = "None";
    QString presetDir;

private slots:
    void valueChanged(int);
    void updateLoadPresetButton();
    void loadSelectedPreset();
    void resetSpinBoxes();
    void savePreset();

private:

    std::vector<QLabel*> funcLabels;
    std::vector<QSpinBox*> spinBoxes;

    QLabel* presetLabel;
    QPushButton* loadPresetButton;
    QPushButton* savePresetButton;
    QComboBox* presetComboBox;
    QDialogButtonBox* buttonBox;
    QPushButton* resetButton;

    ulong nChannels;

    void loadPresets();
    void makeConnections();
};

#endif // FUNCTIONALISATIONDIALOG_H
