#ifndef CLASSSELECTOR_H
#define CLASSSELECTOR_H

#include <QtCore>
#include <QDialog>
#include <QTableWidgetItem>

#include "../classes/aclass.h"
#include "../classes/annotation.h"

namespace Ui {
class ClassSelector;
}

class ClassSelector : public QDialog
{
    Q_OBJECT

public:
    explicit ClassSelector(QWidget *parent = nullptr);
    ~ClassSelector();

    void initClassList();

    Annotation getAnnotation();

    static QList<aClass> getClassList();

    void setSelectedAnnotation(const Annotation &value);

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

    void on_managedClassComboBox_currentTextChanged(const QString &arg1);

    void on_tableWidget_itemChanged(QTableWidgetItem *item);

    void on_annotationTypeComboBox_currentTextChanged(const QString &arg1);

private:
    Ui::ClassSelector *ui;
    Annotation selectedAnnotation;
    static QList<aClass> classList;

    void setButtonsEnabled(bool state);
};

#endif // CLASSSELECTOR_H
