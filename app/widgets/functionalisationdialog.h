#ifndef FUNCTIONALISATIONDIALOG_H
#define FUNCTIONALISATIONDIALOG_H

#include <QtCore>
#include <QDialog>
#include <QSpinBox>
#include <QLabel>
#include <QComboBox>
#include <QDialogButtonBox>

#include "../classes/mvector.h"

class FunctionalisationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FunctionalisationDialog(QWidget *parent = nullptr, ulong nChannels=MVector::nChannels);
    ~FunctionalisationDialog();

    void setFunctionalisation(std::vector<int> funcs);

    std::vector<int> getFunctionalisations();

    QString presetName = "None";

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
