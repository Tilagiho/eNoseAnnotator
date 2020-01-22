#include "attributeeditor.h"
#include "ui_attributeeditor.h"

#include <QtCore>
#include <QMessageBox>

#include "addattributedialog.h"

AttributeEditor::AttributeEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AttributeEditor)
{
    ui->setupUi(this);
}

AttributeEditor::~AttributeEditor()
{
    delete ui;
}

void AttributeEditor::setAttributes(QMap<QString, QString> attributeMap)
{
    attributes = attributeMap;

    ui->tableWidget->setRowCount(attributeMap.size());
    for (int i=0; i<attributeMap.size(); i++)
    {
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem (attributeMap.keys()[i]));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem (attributeMap.values()[i]));
    }
}

void AttributeEditor::on_addButton_clicked()
{
    addAttributeDialog dialog;

    if (dialog.exec())
    {
        QString attribute = dialog.attribute;
        QString defaultValue = dialog.defaultValue;

        if (attributes.contains(attribute))
        {
            QMessageBox::warning(this, "Error: Add Attribute", "Attribute \"" + attribute + "\" already exists!");
            return;
        }

        attributes[attribute] = defaultValue;

        int rowCount = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(rowCount+1);
        ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem(attribute));
        ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem(defaultValue));

        emit addAttribute(attribute, defaultValue);
    }
}

void AttributeEditor::on_deleteButton_clicked()
{

}

void AttributeEditor::on_closeButton_clicked()
{
    accept();
}

void AttributeEditor::on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int, int)
{
    oldValue = ui->tableWidget->item(currentRow, currentColumn)->text();
}

void AttributeEditor::on_tableWidget_cellChanged(int row, int column)
{
    if (column == 0)
    {
        // TODO:
        // rename attribute
    } else
    {
        // TODO:
        // change default value
    }
}
