#ifndef SETSENSORFAILURESDIALOG_H
#define SETSENSORFAILURESDIALOG_H

#include <QtCore>
#include <QDialog>
#include <QCheckBox>

namespace Ui {
class SetSensorFailuresDialog;
}

class SetSensorFailuresDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetSensorFailuresDialog(QWidget *parent = nullptr, QString failureString="");
    ~SetSensorFailuresDialog();
    std::array<bool, 64> getSensorFailures();

private slots:
    void on_buttonBox_accepted();

    void on_ResetButton_clicked();

private:
    Ui::SetSensorFailuresDialog *ui;
    std::array<QCheckBox*, 64> checkBoxArray;
    std::array<bool, 64> sensorFailures;
};

#endif // SETSENSORFAILURESDIALOG_H
