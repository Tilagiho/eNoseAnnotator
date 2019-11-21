#include "infowidget.h"
#include "ui_infowidget.h"

#include <QDateTime>
#include "setsensorfailuresdialog.h"
#include "measurementdata.h"

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

void InfoWidget::setStatus(USBDataSource::Status status)
{
    switch (status)
    {
    case USBDataSource::Status::NOT_CONNECTED:
        ui->statusLabel->setText("Not connected");
        break;
    case USBDataSource::Status::SET_BASELEVEL:
        ui->statusLabel->setText("Setting baselevel...");
        break;
    case USBDataSource::Status::OPEN:
        ui->statusLabel->setText("Receiving data");
        break;
    case USBDataSource::Status::PAUSED:
        ui->statusLabel->setText("Paused");
        break;
    case USBDataSource::Status::CLOSED:
        ui->statusLabel->setText("Closed");
        break;
    case USBDataSource::Status::ERR:
        ui->statusLabel->setText("Error");
        break;
    }

    statusSet = status;
}

void InfoWidget::setTimestamp(double timestamp)
{
    QDateTime dateTime = QDateTime::fromTime_t(timestamp);
    QString timeString = dateTime.toString("d.M.yyyy - h:mm:ss");

    if (timeString != ui->startLabel->text())
        ui->startLabel->setText(timeString);
}

void InfoWidget::setMComment(QString comment)
{
    if (comment != ui->commentTextEdit->toPlainText())
        ui->commentTextEdit->setText(comment);
}

void InfoWidget::setFailures(std::array<bool, 64> failures)
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
        sfDialog = new SetSensorFailuresDialog(this, failureString);

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
