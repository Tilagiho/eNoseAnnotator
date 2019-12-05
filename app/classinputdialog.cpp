#include "classinputdialog.h"

#include <QtCore>
#include <QRegExpValidator>

#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>

ClassInputDialog::ClassInputDialog(QWidget *parent, QString name, QString abreviation) : QDialog(parent)
{
    this->setWindowTitle("Add class");
    QRegExp rx("[\\w| ]*");

    QFormLayout *lytMain = new QFormLayout(this);

    for (int i = 0; i < 2; ++i)
    {
     QLabel *tLabel;
     QLineEdit *tLine = new QLineEdit(this);
     tLine->setValidator(new QRegExpValidator(rx, this));
     if (i==0) {
         tLine->setText(name);
         tLabel = new QLabel("Name:", this);
     }
     else if(i==1) {
         tLine->setText(abreviation);
         tLabel = new QLabel("Abreviation:", this);
     }
     lytMain->addRow(tLabel, tLine);

     fields << tLine;
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox
         ( QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
           Qt::Horizontal, this );
    lytMain->addWidget(buttonBox);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
                this, &ClassInputDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected,
                this, &ClassInputDialog::reject);
    Q_ASSERT(conn);

    setLayout(lytMain);
}

QStringList ClassInputDialog::getStrings(QWidget *parent, QString name, QString abreviation, bool *ok)
{
    ClassInputDialog *dialog = new ClassInputDialog(parent, name, abreviation);

    QStringList list;

    const int ret = dialog->exec();
    if (ok)
        *ok = !!ret;

    if (ret) {
        foreach (auto field, dialog->fields) {
            list << field->text();
        }
    }

    dialog->deleteLater();

    return list;
}
