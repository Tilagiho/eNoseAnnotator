#ifndef ADDATTRIBUTEDIALOG_H
#define ADDATTRIBUTEDIALOG_H

#include <QtCore>
#include <QDialog>
#include "../classes/mvector.h"

namespace Ui {
class addAttributeDialog;
}

class addAttributeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit addAttributeDialog(QWidget *parent = nullptr, QList<QString> attributeList=QList<QString>());
    ~addAttributeDialog();
    void showEvent(QShowEvent *);

    QString attribute;
    QString defaultValue;

private slots:
    void on_attributeLineEdit_editingFinished();

    void on_defaultValueLineEdit_editingFinished();

private:
    Ui::addAttributeDialog *ui;
    QList<QString> attributeList;
    bool initialized = false;
};

#endif // ADDATTRIBUTEDIALOG_H
