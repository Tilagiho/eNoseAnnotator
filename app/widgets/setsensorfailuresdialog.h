#ifndef SETSENSORFAILURESDIALOG_H
#define SETSENSORFAILURESDIALOG_H

#include <QtCore>
#include <QtGui>
#include <QCheckBox>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <vector>

#include "../classes/mvector.h"

namespace Ui {
class SetSensorFailuresDialog;
}

class SetSensorFailuresDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetSensorFailuresDialog(QWidget *parent = nullptr, ulong nChannels = MVector::nChannels, QString failureString="");
    ~SetSensorFailuresDialog();
    std::vector<bool> getSensorFailures();

private slots:
    void resetCheckboxes();

private:
    ulong nChannels;
    std::vector<QCheckBox*> checkBoxes;
    QDialogButtonBox* buttonBox;
    QPushButton* resetButton;

    void makeConnections();
};

#endif // SETSENSORFAILURESDIALOG_H
