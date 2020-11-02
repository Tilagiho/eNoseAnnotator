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

public slots:
    void setSensorId(QString sensor);
    void setComment(QString comment);
    void setSensorFailures(std::vector<bool> failures);
    void setFunctionalisation(Functionalisation &functionalisation);

signals:
    void mCommentChanged(QString comment);
    void setFunctionalitionRequested();
    void setSensorFailuresRequested();

private slots:
    void on_commentTextEdit_textChanged();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
private:
    Ui::InfoWidget *ui;

};

#endif // INFOWIDGET_H
