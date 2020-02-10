#include "classselector.h"
#include "ui_classselector.h"

#include <QMessageBox>
#include <QtCore>
#include <QInputDialog>

#include "../lib/comboboxitemdelegate.h"
#include "classinputdialog.h"

QList<aClass>ClassSelector::classList{};

ClassSelector::ClassSelector(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClassSelector)
{
    ui->setupUi(this);

    initClassList();
    setButtonsEnabled(false);

    // comboBoxDelegate for first column
    ComboBoxItemDelegate* cbid = new ComboBoxItemDelegate(ui->tableWidget);

    // init table
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setRowCount(1);

    ui->tableWidget->setItemDelegateForColumn(0, cbid);
    ui->tableWidget->horizontalHeaderItem(0)->setText("Class");
    ui->tableWidget->horizontalHeaderItem(1)->setText("Value");
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    ui->tableWidget->setColumnHidden(1, true);
}

ClassSelector::~ClassSelector()
{
    delete ui;
}

Annotation ClassSelector::getAnnotation()
{
    return selectedAnnotation;
}

void ClassSelector::initClassList()
{
    ui->managedClassComboBox->clear();
    classList.clear();

    // fill comboBox
    for (aClass c : aClass::staticClassSet)
    {
        ui->managedClassComboBox->addItem(c.toString());
        classList << c;
    }
}


void ClassSelector::on_okButton_clicked()
{
    QList<aClass> selectedClasses;

    // get all selected classes
    for (int i=0; i < ui->tableWidget->rowCount()-1; i++)
    {
        // get name of selected class
        QString className = ui->tableWidget->item(i, 0)->text();

        // get value
        double value = -1.0;
        if (ui->annotationTypeComboBox->currentIndex() == 1)    // type: class + value
        {
            bool isDouble;
            value = ui->tableWidget->item(i, 1)->text().toDouble(&isDouble);

            Q_ASSERT("Value entered is no double!" && isDouble);
        }

        // create class & add to classList
        aClass aclass = aClass(className, value);
        Q_ASSERT("Class already selected!" && !selectedClasses.contains(aclass));
        selectedClasses << aclass;
    }

    selectedAnnotation.set(selectedClasses);

    this->accept();
}

void ClassSelector::on_cancelButton_clicked()
{
    this->reject();
}

void ClassSelector::on_addButton_clicked()
{
    // get new class
    QString name = ClassInputDialog::getName(this);
    if (!name.isEmpty()) {
        aClass newClass(name, -1.0);

        // no empty name or abreviation allowed
        if (newClass.isEmpty())
        {
            QMessageBox::warning(this, "Error", "Class name or abreviation cannot be emtpy!");
            return;
        }

        // check if name or abreviation is new
        for (aClass c : aClass::staticClassSet)
        {
            if (newClass.getName() == c.getName())
            {
                QMessageBox::warning(this, "Error", "Class name " + c.getName() + " already exists!");
                return;
            }
        }
        // add class
        classList << newClass;
        ui->managedClassComboBox->addItem(newClass.toString());

        setButtonsEnabled(true);

        // set current comboBox item to new class
        ui->managedClassComboBox->setCurrentIndex(ui->managedClassComboBox->count()-1);

        emit addClass(newClass);
    }
}

void ClassSelector::on_editButton_clicked()
{
    // get selected class
    aClass oldClass = aClass::fromString(ui->managedClassComboBox->currentText());

    Q_ASSERT("Selected class has to be in classList!" && aClass::staticClassSet.contains(oldClass));

    // get new class
    QString newName = ClassInputDialog::getName(this, oldClass.getName());
    if (!newName.isEmpty()) {
        aClass newClass(newName);

        // no empty name or abreviation allowed
        if (newClass.isEmpty())
        {
            QMessageBox::warning(this, "Error", "Class name or abreviation cannot be emtpy!");
            return;
        }
        // no changes to class: do nothing
        if (oldClass == newClass)
            return;

        // make sure that name and abreviation are new
        for (aClass c : aClass::staticClassSet)
        {
            // ignore old class
            if (c == oldClass)
                continue;

            if (newClass.getName() == c.getName())
            {
                QMessageBox::warning(this, "Error", "Class name " + c.getName() + " already exists!");
                return;
            }
        }

        // question: make sure measurement data should be changed
        QString questionText = "Are you sure you want to change " + oldClass.toString() + " to " + newClass.toString() + "?\n This will change all instances of " + oldClass.toString() + " in the measurement data.";
        auto answer = QMessageBox::question(this, "Changing class", questionText);
        if (answer != QMessageBox::StandardButton::Yes)
            return;

        // change comboBox text
        int index = ui->managedClassComboBox->findText(oldClass.toString());
        ui->managedClassComboBox->setItemText(index, newClass.toString());

        // emit change
        emit changeClass(oldClass, newClass);
    }
}

void ClassSelector::on_deleteButton_clicked()
{
    // get current class
    aClass currentClass = aClass::fromString(ui->managedClassComboBox->currentText());
    Q_ASSERT("Class list has to contain current class!" && aClass::staticClassSet.contains(currentClass));

    // question: make sure measurement data should be changed
    QString questionText = "Are you sure you want to delete " + currentClass.toString() + "?\n This will delete all instances of " + currentClass.toString() + " in the measurement data.";
    auto answer = QMessageBox::question(this, "Deleting class", questionText);
    if (answer != QMessageBox::StandardButton::Yes)
        return;

    // delete current class
    int index = ui->managedClassComboBox->findText(currentClass.toString());
    ui->managedClassComboBox->removeItem(index);

    if (ui->managedClassComboBox->count() == 0)
    {
        setButtonsEnabled(false);
    }

    // emit deletion
    emit removeClass(currentClass);
}

void ClassSelector::setButtonsEnabled(bool state)
{
    ui->okButton->setEnabled(state);
    ui->editButton->setEnabled(state);
    ui->deleteButton->setEnabled(state);
}

void ClassSelector::on_managedClassComboBox_currentTextChanged(const QString &arg1)
{
    if (ui->managedClassComboBox->count() > 0)
        setButtonsEnabled(true);
}

QList<aClass> ClassSelector::getClassList()
{
    return classList;
}

void ClassSelector::on_tableWidget_itemChanged(QTableWidgetItem *item)
{
    if (item->column() == 0)
    {
        // handling the number of rows:
        // no class selected and not last row:
        // remove row
        if (item->text().isEmpty() && item->row() != ui->tableWidget->rowCount()-1)
            ui->tableWidget->removeRow(item->row());

        // last row was edited && class selected:
        // increase number of rows
        else if (!item->text().isEmpty() && item->row() == ui->tableWidget->rowCount()-1)
        {
            ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
            auto valueItem = new QTableWidgetItem();
            valueItem->setData(Qt::EditRole, 1.0);
            ui->tableWidget->setItem(item->row(), 1, valueItem);
        }

        // repopulate classList:
        // clear
        classList.clear();

        // init with all classes
        classList = aClass::staticClassSet.toList();

        // remove selected classes
        for (int i=0; i<ui->tableWidget->rowCount()-1; i++)
        {
            aClass c = aClass::fromString(ui->tableWidget->item(i, 0)->text());

            Q_ASSERT(classList.contains(c));
            classList.removeAll(c);
        }

        // set status of acceptance button
        ui->okButton->setEnabled(ui->tableWidget->rowCount() > 1);
    } else if (item->column() == 1)
    {
        // last row changed (no class selected for value)
        // -> ignore changes
        if (item->row() == ui->tableWidget->rowCount()-1 && item->text() != "")
        {
            auto valueItem = new QTableWidgetItem();
            valueItem->setData(Qt::EditRole, "");
            ui->tableWidget->setItem(item->row(), 1, valueItem);
        }
        // value deleted
        // -> set to 0.0
        else if (item->row() != ui->tableWidget->rowCount()-1 && item->text() == "")
        {
            auto valueItem = new QTableWidgetItem();
            valueItem->setData(Qt::EditRole, 0.0);
            ui->tableWidget->setItem(item->row(), 1, valueItem);
        }
    }
}

void ClassSelector::on_annotationTypeComboBox_currentTextChanged(const QString &text)
{
    if (text == "Class")
        ui->tableWidget->setColumnHidden(1, true);
    else
        ui->tableWidget->setColumnHidden(1, false);
}

void ClassSelector::setSelectedAnnotation(const Annotation &value)
{
    selectedAnnotation = value;

    // populate tableWidget
    auto annotationClasses = selectedAnnotation.getClasses();
    for (int i=0; i<annotationClasses.size(); i++)
    {
        aClass c = annotationClasses[i];

        auto classItem = new QTableWidgetItem();
        classItem->setData(Qt::EditRole, c.getName());
        ui->tableWidget->setItem(i, 0, classItem);

        auto valueItem = new QTableWidgetItem();
        double value = c.getValue() < 0.0 ? 1.0 : c.getValue(); // class only values are set to 1.0
        valueItem->setData(Qt::EditRole, value);
        ui->tableWidget->setItem(i, 1, valueItem);
    }

    if (selectedAnnotation.getType() == aClass::Type::CLASS_ONLY)
    {
        ui->annotationTypeComboBox->setCurrentIndex(0);
        ui->tableWidget->hideColumn(1);

    } else if (selectedAnnotation.getType() == aClass::Type::NUMERIC)
    {
        ui->annotationTypeComboBox->setCurrentIndex(1);
        ui->tableWidget->showColumn(1);
    }

}
