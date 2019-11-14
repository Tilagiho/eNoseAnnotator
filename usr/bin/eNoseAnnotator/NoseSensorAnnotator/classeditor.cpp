#include "classeditor.h"
#include "ui_classeditor.h"

#include <QtCore>
#include <QInputDialog>
#include <QMessageBox>

ClassEditor::ClassEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClassEditor)
{
    ui->setupUi(this);

    ui->deleteButton->setEnabled(false);
    ui->renameButton->setEnabled(false);
}

ClassEditor::~ClassEditor()
{
    delete ui;
}

QStringList ClassEditor::getClassList() const
{
    return classList;
}

void ClassEditor::setClassList(const QStringList &value)
{
    ui->listWidget->clear();
    ui->listWidget->addItems(value);
    classList = value;
}

void ClassEditor::on_addButton_clicked()
{
    QString className;

    bool ok;
    className = QInputDialog::getText(this, "Add New Class", "Class name: ", QLineEdit::Normal, className, &ok);

    if (!ok || className=="")
        return;
    if (classList.contains(className) )
        QMessageBox::warning(this ,"Invalid class name", "Class name" + className + "already exists!");
    else if (className.contains(";") || className.contains("\n"))
        QMessageBox::warning(this ,"Invalid class name", "Characters \";\" and \"\\n\" are not allowed in class names!");

    classList << className;
    ui->listWidget->addItem(className);
}

void ClassEditor::on_deleteButton_clicked()
{
    int currentRow = ui->listWidget->currentRow();
    QString className = ui->listWidget->currentItem()->text();

    auto answer = QMessageBox::question(this, "Delete class", "Are you sure you want to delete class \"" + className + "\"?\nThis will declassify all annotations with this class.");

    if (answer == QMessageBox::StandardButton::Yes)
    {
        int currentRow = ui->listWidget->currentRow();
        ui->listWidget->takeItem(currentRow);
        classList.takeAt(currentRow);

        emit deleteClass(className);
    }

}

void ClassEditor::on_renameButton_clicked()
{
    QString oldName = ui->listWidget->currentItem()->text();
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename class", "Rename class \"" + oldName + "\" to:", QLineEdit::Normal, oldName, &ok);

    if (!ok || newName == "")
        return;
    if (classList.contains(newName))
        QMessageBox::warning(this ,"Invalid class name", "Class name \"" + newName + "\" already exists!");
    else if (newName.contains(";") || newName.contains("\n"))
        QMessageBox::warning(this ,"Invalid class name", "Characters \";\" and \"\\n\" are not allowed in class names!");
    else    // new valid name
    {
        auto answer = QMessageBox::question(this, "Rename class", "Are you sure you want to rename class \"" + oldName + "\" to \"" + newName + "\"?\nThis will reclassify all annotations with this class.");

        if (answer == QMessageBox::StandardButton::Yes)
        {
            classList[ui->listWidget->currentRow()] = newName;
            ui->listWidget->currentItem()->setText(newName);

            emit renameClass(oldName, newName);
        }
    }
}

void ClassEditor::on_listWidget_currentRowChanged(int currentRow)
{
    if (currentRow == -1)
    {
        ui->renameButton->setEnabled(false);
        ui->deleteButton->setEnabled(false);
    } else
    {
        ui->renameButton->setEnabled(true);
        ui->deleteButton->setEnabled(true);
    }
}

void ClassEditor::on_closeButton_clicked()
{
    this->accept();
}
