#ifndef CLASSINPUTDIALOG_H
#define CLASSINPUTDIALOG_H

#include <QDialog>

class QLineEdit;
class QLabel;

class ClassInputDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ClassInputDialog(QWidget *parent = nullptr, QString name="", QString abreviation="");

    static QStringList getStrings(QWidget *parent, QString name="", QString abreviation="", bool *ok = nullptr);

private:
    QList<QLineEdit*> fields;
};

#endif // CLASSINPUTDIALOG_H
