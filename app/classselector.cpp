#include "classselector.h"
#include "ui_classselector.h"

#include "classinputdialog.h"
#include <QMessageBox>
#include <QtCore>
#include <QInputDialog>

ClassSelector::ClassSelector(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClassSelector)
{
    ui->setupUi(this);

    setButtonsEnabled(false);
}

ClassSelector::~ClassSelector()
{
    delete ui;
}

aClass ClassSelector::getClass()
{
    return selectedClass;
}

void ClassSelector::setClassList(const QList<aClass> &value)
{
    ui->comboBox->clear();

    // add "None": used to remove class
    classList.append(aClass{"",""});
    ui->comboBox->addItem("~None~");

    for (aClass c : value)
        ui->comboBox->addItem(c.toString());

    classList.append(value);
}


void ClassSelector::on_okButton_clicked()
{
    QString selectedString = ui->comboBox->currentText();

    if (selectedString == "~None~")
        selectedClass = aClass{"",""};
    else
        selectedClass = aClass::fromString(selectedString);

    Q_ASSERT("Selected class is not in classList!" && (selectedClass.isEmpty() || classList.contains(selectedClass)));

    this->accept();
}

void ClassSelector::on_cancelButton_clicked()
{
    this->reject();
}

void ClassSelector::on_addButton_clicked()
{
    // get new class
    QStringList list = ClassInputDialog::getStrings(this);
    if (!list.isEmpty()) {
        aClass newClass(list[0], list[1]);

        // no empty name or abreviation allowed
        if (newClass.isEmpty())
        {
            QMessageBox::warning(this, "Error", "Class name or abreviation cannot be emtpy!");
            return;
        }

        // check if name or abreviation is new
        for (aClass c : classList)
        {
            if (newClass.getName() == c.getName())
            {
                QMessageBox::warning(this, "Error", "Class name " + c.getName() + " already exists!");
                return;
            }
            if (newClass.getAbreviation() == c.getAbreviation())
            {
                QMessageBox::warning(this, "Error", "Class abreviation " + c.getAbreviation() + " already exists!");
                return;
            }
        }
        // add class
        classList.append(newClass);
        ui->comboBox->addItem(newClass.toString());

        setButtonsEnabled(true);

        // set current comboBox item to new class
        int index = classList.indexOf(newClass);
        ui->comboBox->setCurrentIndex(index);

        emit addClass(newClass);
    }
}

void ClassSelector::on_editButton_clicked()
{
    // get selected class
    aClass oldClass = aClass::fromString(ui->comboBox->currentText());

    Q_ASSERT("Selected class has to be in classList!" && classList.contains(oldClass));

    // get new class
    QStringList list = ClassInputDialog::getStrings(this, oldClass.getName(), oldClass.getAbreviation());
    if (!list.isEmpty()) {
        aClass newClass(list[0], list[1]);

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
        for (aClass c : classList)
        {
            // ignore old class
            if (c == oldClass)
                continue;

            if (newClass.getName() == c.getName())
            {
                QMessageBox::warning(this, "Error", "Class name " + c.getName() + " already exists!");
                return;
            }
            if (newClass.getAbreviation() == c.getAbreviation())
            {
                QMessageBox::warning(this, "Error", "Class abreviation " + c.getAbreviation() + " already exists!");
                return;
            }
        }

        // question: make sure measurement data should be changed
        QString questionText = "Are you sure you want to change " + oldClass.toString() + " to " + newClass.toString() + "?\n This will change all instances of " + oldClass.toString() + " in the measurement data.";
        auto answer = QMessageBox::question(this, "Changing class", questionText);
        if (answer != QMessageBox::StandardButton::Yes)
            return;

        // change old class int new class
        int index = classList.indexOf(oldClass);
        classList[index] = newClass;
        ui->comboBox->setItemText(index, newClass.toString());

        // emit change
        emit changeClass(oldClass, newClass);
    }
}

void ClassSelector::on_deleteButton_clicked()
{
    // get current class
    aClass currentClass = aClass::fromString(ui->comboBox->currentText());
    Q_ASSERT("Class list has to contain current class!" && classList.contains(currentClass));


    // question: make sure measurement data should be changed
    QString questionText = "Are you sure you want to delete " + currentClass.toString() + "?\n This will delete all instances of " + currentClass.toString() + " in the measurement data.";
    auto answer = QMessageBox::question(this, "Deleting class", questionText);
    if (answer != QMessageBox::StandardButton::Yes)
        return;

    // delete current class
    int index = classList.indexOf(currentClass);
    classList.removeAt(index);
    ui->comboBox->removeItem(index);

    if (classList.isEmpty())
    {
        setButtonsEnabled(false);
    }

    // emit deletion
    emit removeClass(currentClass);
}

void ClassSelector::on_comboBox_currentTextChanged(const QString &arg1)
{
    if (arg1 == "~None~")
    {
        // disable all buttons except for okButton
        setButtonsEnabled(false);
        ui->okButton->setEnabled(true);
    }
    else
        setButtonsEnabled(true);
}

void ClassSelector::setButtonsEnabled(bool state)
{
    ui->okButton->setEnabled(state);
    ui->editButton->setEnabled(state);
    ui->deleteButton->setEnabled(state);
}
