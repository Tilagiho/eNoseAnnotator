#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QtCore>
#include <QWidget>

#include "../classes/mvector.h"
#include "../classes/datasource.h"

namespace Ui {
class InfoWidget;
}

class InfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InfoWidget(QWidget *parent = nullptr);
    ~InfoWidget();

    DataSource::Status statusSet = DataSource::Status::NOT_CONNECTED;

    QString getFuncLabel();

public slots:
    void setSensor(QString sensor);
    void setStatus(DataSource::Status status);
    void setTimestamp(double timestamp);
    void setMComment(QString comment);
    void setFailures(std::vector<bool> failures);
    void setFuncLabel(QString label);

signals:
    void mCommentChanged(QString comment);
    void failuresChanged(std::vector<bool> failures);
    void setFunctionalitionClicked();
    void setSensorFailuresClicked();

private slots:
    void on_commentTextEdit_textChanged();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
private:
    Ui::InfoWidget *ui;

};

#endif // INFOWIDGET_H
