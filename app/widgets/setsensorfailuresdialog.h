#ifndef SETSENSORFAILURESDIALOG_H
#define SETSENSORFAILURESDIALOG_H

#include <QtCore>
#include <QtGui>
#include <QCheckBox>
#include <QDialog>
#include <array>
#include <vector>

namespace Ui {
class SetSensorFailuresDialog;
}

class SetSensorFailuresDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetSensorFailuresDialog(QWidget *parent = nullptr, QString failureString="");
    ~SetSensorFailuresDialog();
    std::vector<bool> getSensorFailures();

private slots:
    void on_buttonBox_accepted();

    void on_ResetButton_clicked();

private:
    Ui::SetSensorFailuresDialog *ui;
    std::vector<QCheckBox*> checkBoxes;
    std::vector<bool> sensorFailures;
};

#endif // SETSENSORFAILURESDIALOG_H
