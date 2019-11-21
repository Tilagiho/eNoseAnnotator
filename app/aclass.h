#ifndef ACLASS_H
#define ACLASS_H

#include <QtCore>

/*
 * Class of vector
 */
class aClass
{
public:
    aClass(QString name, QString abreviation);

    bool isEmpty();

    void setClass(QString name, QString abreviation);

    QString getName() const;

    QString getAbreviation() const;

    QString toString();

    static aClass fromString(QString string);

    bool operator ==(const aClass &b) const;

    bool operator !=(const aClass &b) const;

private:

    QString name;
    QString abreviation;
};

#endif // ACLASS_H
