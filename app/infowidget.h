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
    void setFailures(std::array<bool, 64> failures);

signals:
    void mCommentChanged(QString comment);
    void failuresChanged(std::array<bool, 64> failures);
    void setFunctionalities();

private slots:
    void on_commentTextEdit_textChanged();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
private:
    Ui::InfoWidget *ui;

};

#endif // INFOWIDGET_H
