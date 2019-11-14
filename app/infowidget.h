#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QtCore>
#include <QWidget>

#include "mvector.h"
#include "usbdatasource.h"

namespace Ui {
class InfoWidget;
}

class InfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InfoWidget(QWidget *parent = nullptr);
    ~InfoWidget();

    USBDataSource::Status statusSet = USBDataSource::Status::NOT_CONNECTED;


public slots:
    void setSensor(QString sensor);
    void setStatus(USBDataSource::Status status);
    void setTimestamp(double timestamp);
    void setMComment(QString comment);
    void setDComment(QString comment);
    void setFailures(std::array<bool, 64> failures);
    void showAddSelectionButton();
    void hideAddSelectionButton();
    void setNEntries(uint n);
    void setNClasses(uint n);

signals:
    void mCommentChanged(QString comment);
    void dCommentChanged(QString comment);
    void failuresChanged(std::array<bool, 64> failures);
    void addSelection();
    void editAnnotationData();
    void setFunctionalities();

private slots:
    void on_commentTextEdit_textChanged();

    void on_pushButton_clicked();

    void on_addSelectionButton_clicked();

    void on_editDatasetButton_clicked();



    void on_pushButton_2_clicked();

    void on_datasetNameLineEdit_textChanged(const QString &arg1);

private:
    Ui::InfoWidget *ui;

};

#endif // INFOWIDGET_H
