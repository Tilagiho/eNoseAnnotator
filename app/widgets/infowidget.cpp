#include "infowidget.h"
#include "ui_infowidget.h"

#include <QDateTime>

#include "setsensorfailuresdialog.h"
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

void InfoWidget::setSensor(QString sensor)
{
    if (sensor != ui->sensorLabel->text())
        ui->sensorLabel->setText(sensor);
}

void InfoWidget::setStatus(DataSource::Status status)
{
    switch (status)
    {
    case DataSource::Status::NOT_CONNECTED:
        ui->statusLabel->setText("Not connected");
        break;
    case DataSource::Status::CONNECTING:
        ui->statusLabel->setText("Connecting...");
        break;
    case DataSource::Status::CONNECTED:
        ui->statusLabel->setText("Connected");
        break;
    case DataSource::Status::SET_BASEVECTOR:
        ui->statusLabel->setText("Setting Base Vector (R0)...");
        break;
    case DataSource::Status::RECEIVING_DATA:
        ui->statusLabel->setText("Receiving data");
        break;
    case DataSource::Status::CONNECTION_ERROR:
        ui->statusLabel->setText("Error");
        break;
    case DataSource::Status::PAUSED:
        ui->statusLabel->setText("Paused");
        break;
    }

    statusSet = status;
}

void InfoWidget::setTimestamp(double timestamp)
{
    QString timeString = MeasurementData::getTimestampStringFromUInt(timestamp);

    if (timeString != ui->startLabel->text())
        ui->startLabel->setText(timeString);
}

void InfoWidget::setMComment(QString comment)
{
    if (comment != ui->commentTextEdit->toPlainText())
        ui->commentTextEdit->setText(comment);
}

void InfoWidget::setFailures(std::vector<bool> failures)
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
    SetSensorFailuresDialog *sfDialog;
    QString failureString = ui->failureLabel->text();

    if (failureString == "None")
        sfDialog = new SetSensorFailuresDialog(this);
    else
        sfDialog = new SetSensorFailuresDialog(this, MVector::nChannels, failureString);

    sfDialog->setWindowTitle("Sensor failure flags");

    if(sfDialog->exec())
    {
        emit failuresChanged(sfDialog->getSensorFailures());
    }
}

void InfoWidget::on_pushButton_2_clicked()
{
    emit setFunctionalities();
}

void InfoWidget::setFuncLabel(QString label)
{
    if (label.endsWith(".preset"))
        label = label.left(label.size()-QString(".preset").size());
    ui->funcLabel->setText(label);
}

QString InfoWidget::getFuncLabel()
{
    return ui->funcLabel->text();
}
