#include "classinputdialog.h"

#include <QtCore>
#include <QRegExpValidator>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>

ClassInputDialog::ClassInputDialog(QWidget *parent, QString name) : QDialog(parent)
{
    this->setWindowTitle("Add class");
    QRegExp rx("[\\w _\\-]*");

    QFormLayout *lytMain = new QFormLayout(this);

     QLabel *tLabel;
     QLineEdit *tLine = new QLineEdit(this);
     tLine->setValidator(new QRegExpValidator(rx, this));

     tLine->setText(name);
     tLabel = new QLabel("Name:", this);

     lytMain->addRow(tLabel, tLine);

     fields << tLine;

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

QString ClassInputDialog::getName(QWidget *parent, QString name, bool *ok)
{
    ClassInputDialog *dialog = new ClassInputDialog(parent, name);

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

    if (ret)
        return list[0];
    else
        return "";
}
