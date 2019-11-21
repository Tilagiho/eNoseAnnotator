#ifndef CLASSSELECTOR_H
#define CLASSSELECTOR_H

#include <QtCore>
#include <QDialog>
#include "aclass.h"

namespace Ui {
class ClassSelector;
}

class ClassSelector : public QDialog
{
    Q_OBJECT

public:
    explicit ClassSelector(QWidget *parent = nullptr);
    ~ClassSelector();

    void setClassList(const QList<aClass> &value);

    aClass getClass();

signals:
    void addClass(aClass newClass);
    void removeClass(aClass oldClass);
    void changeClass (aClass oldClass, aClass newClass);

private slots:

    void on_okButton_clicked();

    void on_cancelButton_clicked();

    void on_addButton_clicked();

    void on_editButton_clicked();

    void on_deleteButton_clicked();

    void on_comboBox_currentTextChanged(const QString &arg1);

private:
    Ui::ClassSelector *ui;

    QList<aClass> classList;
    aClass selectedClass{"",""};

    void setButtonsEnabled(bool state);
};

#endif // CLASSSELECTOR_H
