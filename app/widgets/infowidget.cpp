#include "infowidget.h"
#include "ui_infowidget.h"

#include <QDateTime>

#include "../classes/measurementdata.h"

InfoWidget::InfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoWidget)
{
    ui->setupUi(this);
}

InfoWidget::~InfoWidget()
{
    delete ui;
}

void InfoWidget::setSensorId(QString sensor)
{
    if (sensor != ui->sensorLabel->text())
        ui->sensorLabel->setText(sensor);
}

void InfoWidget::setComment(QString comment)
{
    if (comment != ui->commentTextEdit->toPlainText())
        ui->commentTextEdit->setText(comment);
}

void InfoWidget::setSensorFailures(std::vector<bool> failures)
{

    QString label = MeasurementData::sensorFailureString(failures);

    if (label != ui->failureLabel->text())
        ui->failureLabel->setText(label);
}

void InfoWidget::on_commentTextEdit_textChanged()
{
    QString commentText = ui->commentTextEdit->toPlainText();

    if (commentText.contains(';') )
    {
        // erase ';' from commentText
        commentText = commentText.split(';').join("");
    }

    emit mCommentChanged(commentText);
}

void InfoWidget::on_pushButton_clicked()
{
    emit setSensorFailuresRequested();
}

void InfoWidget::on_pushButton_2_clicked()
{
    emit setFunctionalitionRequested();
}

void InfoWidget::setFunctionalisation(Functionalisation &functionalisation)
{
    QString label = functionalisation.getName();
    if (label.endsWith(".preset"))
        label = label.left(label.size()-QString(".preset").size());
    ui->funcLabel->setText(label);
}
