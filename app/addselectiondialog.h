#ifndef ADDSELECETIONDIALOG_H
#define ADDSELECETIONDIALOG_H

#include <QtCore>
#include <QDialog>
#include <QMap>
#include "annotation_old.h"
#include "mvector.h"
#include <QTableWidgetItem>

namespace Ui {
class AddSelecetionDialog;
}

class AddSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddSelectionDialog(QWidget *parent = nullptr);
    ~AddSelectionDialog();

    void setVector(QMap<uint, MVector> vectorMap);

    void setBaseLevel(MVector baseLevel);

    void setFailureBits(const std::array<bool, 64> value);

    void setNVectors(const uint value);

    void setExtraAttributes(const QMap<QString, QString> value);

    void setClassNames(const QList<QString> value);

    Annotation getAnnotation() const;

    void setComment(const QString value);

    void setSensorId(const QString value);

    void setTimestamp(double value);

signals:
    void extraAttributeAdded(QString attribute, QString defaultValue);
    void extraAttributeRenamed (QString oldName, QString newName);
    void classAdded(QString className);

private slots:
    void on_addAttributeButton_clicked();

    void on_tableWidget_itemChanged(QTableWidgetItem *item);

    void on_pushButton_clicked();


    void on_tableWidget_itemActivated(QTableWidgetItem *item);

private:
    Ui::AddSelecetionDialog *ui;

    QString sensorId;
    double timestamp;
    MVector vector;
    MVector baseLevelVector;
    QString sensorFailureString;
    uint nVectors;
    QMap<QString, QString> extraAttributes;
    QList<QString> classNames;
    QString comment;

    QString oldTableViewValue;   // used to save values before they are changed in the table view

    void addExtraAttributeToTable(QString name, QString value);
    void addClass (QString className);

};

#endif // ADDSELECETIONDIALOG_H
