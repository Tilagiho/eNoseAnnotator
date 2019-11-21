#ifndef EDITANNOTATIONDATAWINDOW_H
#define EDITANNOTATIONDATAWINDOW_H

#include <QtCore>
#include <QMainWindow>
#include "annotationdatasetmodel_old.h"
#include "mvector.h"

namespace Ui {
class editAnnotationDataWindow;
}

class editAnnotationDataWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit editAnnotationDataWindow(QWidget *parent = nullptr);
    ~editAnnotationDataWindow();

    void setModel(QAbstractItemModel* model);

signals:

private slots:
    void on_actionLoad_triggered();

    void on_actionSave_triggered();

    void on_actionEditAttributes_triggered();

    void on_actionEditClasses_triggered();

private:
    Ui::editAnnotationDataWindow *ui;
    AnnotationDatasetModel* aData = nullptr;
};

#endif // EDITANNOTATIONDATAWINDOW_H
