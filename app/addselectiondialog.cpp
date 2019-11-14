#include "addselectiondialog.h"
#include "ui_addselectiondialog.h"

#include "addattributedialog.h"
#include "measurementdata.h"
#include "mvector.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>

AddSelectionDialog::AddSelectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddSelecetionDialog)
{
    ui->setupUi(this);

    // set validator for comments
    ui->commentLineEdit->setValidator(new QRegExpValidator(QRegExp("[\\w| |,|:]*")));

    // set multi modes
    for (auto mode : MeasurementData::getMultiModeList())
        ui->multiModeComboBox->addItem(MeasurementData::multiModeToQString(mode));
    // default mode: Average
    ui->multiModeComboBox->setCurrentText(MeasurementData::multiModeToQString(MeasurementData::MultiMode::Average));
}

AddSelectionDialog::~AddSelectionDialog()
{
    delete ui;
}

void AddSelectionDialog::addExtraAttributeToTable(QString name, QString value)
{
    int row = (ui->tableWidget->rowCount());
    ui->tableWidget->setRowCount(row+1);

    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(name));
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(value));

    ui->tableWidget->update();
}

void AddSelectionDialog::on_addAttributeButton_clicked()
{
    addAttributeDialog *dialog = new addAttributeDialog(this, extraAttributes.keys());

    if (dialog->exec())
    {
        QString attribute = dialog->attribute;
        QString defaultValue = dialog->defaultValue;

        // make sure attribute name is new
        if (extraAttributes.contains(attribute))
            QMessageBox::warning(this, "Adding attribute not successfull", "Attribute name \"" + attribute +"\" already exists.");
        else
        {
            addExtraAttributeToTable(attribute, defaultValue);

            extraAttributes[attribute] = defaultValue;
            emit extraAttributeAdded(attribute, defaultValue);
        }
    }
}

void AddSelectionDialog::setComment(const QString value)
{
    comment = value;
    ui->commentLineEdit->setText(value);
}

Annotation AddSelectionDialog::getAnnotation() const
{
    MeasurementData::MultiMode multiMode = MeasurementData::qStringToMultiMode(ui->multiModeComboBox->currentText());
    Annotation annotation (ui->classComboBox->currentText(), timestamp, vector, baseLevelVector, sensorId, ui->commentLineEdit->text(), nVectors, extraAttributes.values(), multiMode, sensorFailureString);
    return annotation;
}

void AddSelectionDialog::setClassNames(const QList<QString> value)
{
    ui->classComboBox->addItems(value);
    classNames = value;
}

void AddSelectionDialog::setExtraAttributes(const QMap<QString, QString> value)
{
    // iterate through map and set items
    auto iter = value.begin();

    while (iter != value.end())
    {
        addExtraAttributeToTable(iter.key(), iter.value());
        iter++;
    }
    extraAttributes = value;
}

void AddSelectionDialog::setNVectors(const uint value)
{
    nVectors = value;
    ui->nVectorsLabel->setText(QString::number(value));
}

void AddSelectionDialog::setFailureBits(const std::array<bool, 64> value)
{
    sensorFailureString = MeasurementData::sensorFailureString(value);
    ui->failureLabel->setText(sensorFailureString);
}

void AddSelectionDialog::setVector(QMap<uint, MVector> vectorMap)
{
    auto beginIter = vectorMap.begin();
    auto endIter = vectorMap.end();

    vector = MeasurementData::getSelectionVector(beginIter, endIter);
    ui->bGraph->setBars(vector);
}

void AddSelectionDialog::setBaseLevel(MVector baseLevel)
{
    baseLevelVector = baseLevel;
}

void AddSelectionDialog::on_tableWidget_itemChanged(QTableWidgetItem *item)
{
        QString itemText = item->text();

        if (item->column() == 0)    // attribute changed
        {
            // if empty filed: restore oldTableView
            if (itemText == "")
            {
                item->setText(oldTableViewValue);
                return;
            }
            else if (oldTableViewValue != "")
                // else: rename edited attribute
                emit extraAttributeRenamed(oldTableViewValue, itemText);

        } else  // value changed: update value for attribute
        {
            QString attribute = ui->tableWidget->item(item->row(), 0)->text();

            extraAttributes[attribute] = itemText;
        }
}

void AddSelectionDialog::on_pushButton_clicked()
{
    bool inputOk;
    QString new_class = QInputDialog::getText(this, "Create new class", "Insert new class:", QLineEdit::Normal, "", &inputOk);

    QRegExp re("[\\w| ]*");

    QString warning;
    if (!re.exactMatch(new_class))
        warning = "Class name can only contain alpha-numeric characters and \" \"";
    else if (classNames.contains(new_class))
        warning = "Class \"" + new_class + "\" already exists!";

    if (!warning.isEmpty())
        QMessageBox::critical(this, "Class can not be added", warning);
    else
        addClass(new_class);
}

void AddSelectionDialog::setTimestamp(double value)
{
    timestamp = value;
    ui->timestempLabel->setText(QDateTime::fromTime_t(value).toString("d.M.yyyy - h:mm:ss"));
}

void AddSelectionDialog::setSensorId(const QString value)
{
    sensorId = value;
    ui->sensorLabel->setText(value);
}

void AddSelectionDialog::addClass(QString name)
{
    Q_ASSERT(!classNames.contains(name));

    classNames.append(name);
    ui->classComboBox->addItem(name);
    emit classAdded(name);
}

void AddSelectionDialog::on_tableWidget_itemActivated(QTableWidgetItem *item)
{
    oldTableViewValue = item->text();
}
