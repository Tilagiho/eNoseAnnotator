#include "addattributedialog.h"
#include "ui_addattributedialog.h"

#include <QMessageBox>
#include <QPushButton>
#include <QRegExpValidator>

addAttributeDialog::addAttributeDialog(QWidget *parent, QList<QString> attributeList) :
    QDialog(parent),
    ui(new Ui::addAttributeDialog),
    attributeList(attributeList)
{
    ui->setupUi(this);
    this->setWindowTitle("Add New Attribute");

    // set validators for line edits
    ui->attributeLineEdit->setValidator(new QRegExpValidator(QRegExp("[\\w| ]*")));
    ui->defaultValueLineEdit->setValidator(new QRegExpValidator(QRegExp("[\\w| ]*")));

}

void addAttributeDialog::showEvent(QShowEvent *)
{
    if (!initialized)
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        initialized = true;
    }
}

addAttributeDialog::~addAttributeDialog()
{
    delete ui;
}

void addAttributeDialog::on_attributeLineEdit_editingFinished()
{
    QRegExp re("\\w*");
    QString lineString =  ui->attributeLineEdit->text();
    if (re.exactMatch(lineString) && !attributeList.contains(lineString))
    {
        attribute = lineString;
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    else
    {
        QString message;

        if (attributeList.contains(lineString))
            message = "Attribute \"" + lineString + "\" already exists!";
        else
            message = "Attribute name has to be alpha-numeric!";
        QMessageBox::critical(this, "Error", message);

        ui->attributeLineEdit->setText("");
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

void addAttributeDialog::on_defaultValueLineEdit_editingFinished()
{
    QRegExp re("\\w*");
    QString lineString =  ui->defaultValueLineEdit->text();
    if (re.exactMatch(lineString))
        defaultValue = lineString;
    else
    {
        QMessageBox::critical(this, "Error", "Default Value has to be alpha-numeric!");
        ui->defaultValueLineEdit->setText("");
    }
}
