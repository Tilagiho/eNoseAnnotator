#ifndef FUNCTIONALISATIONDIALOG_H
#define FUNCTIONALISATIONDIALOG_H

#include <QtCore>
#include "mvector.h"
#include <QDialog>
#include <QSpinBox>

namespace Ui {
class FunctionalisationDialog;
}

class FunctionalisationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FunctionalisationDialog(QWidget *parent = nullptr);
    ~FunctionalisationDialog();

    void setFunctionalities(std::array<int, 64> funcs);

    std::array<int, 64> getFunctionalities();

private slots:
    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::FunctionalisationDialog *ui;

    // stores spinboxes
    std::array<QSpinBox*, 64> spArray;
};

#endif // FUNCTIONALISATIONDIALOG_H
