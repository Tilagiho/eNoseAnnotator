#ifndef SETSOURCEDIALOG_H
#define SETSOURCEDIALOG_H

#include <QDialog>
#include "../classes/datasource.h"
#include "../classes/usbdatasource.h"
#include "usbsettingswidget.h"

namespace Ui {
class SourceDialog;
}

class SourceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SourceDialog(QWidget *parent = nullptr);
    ~SourceDialog();

    QString getIdentifier();

    DataSource::SourceType getSourceType() const;
    void setSourceType(const DataSource::SourceType &value);

    void setUSBSettings(USBDataSource::Settings usbSettings);



    QString getSensorId() const;
    void setSensorId(const QString &value);

private slots:
    void on_cancelButton_clicked();

    void on_applyButton_clicked();

    void on_sensorIdLineEdit_textEdited(const QString &arg1);

    void on_comboBox_currentTextChanged(const QString &arg1);

private:
    Ui::SourceDialog *ui;

    USBSettingsWidget* usbWidget;
    USBSettingsWidget* fakeWidget;

    DataSource::SourceType sourceType;
    QString identifier;
    QString sensorId;

};

#endif // SETSOURCEDIALOG_H
