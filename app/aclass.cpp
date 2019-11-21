#include "aclass.h"

#include <QtCore>

aClass::aClass(QString className, QString classAbreviation):
    name{className},
    abreviation{classAbreviation}
{}

void aClass::setClass(QString className, QString classAbreviation)
{
    this->name = className;
    this->abreviation = classAbreviation;
}

bool aClass::isEmpty()
{
    return (name == "") || (abreviation == "");
}

QString aClass::getName() const
{
    return name;
}

QString aClass::getAbreviation() const
{
    return abreviation;
}

QString aClass::toString()
{
    if (isEmpty())
        return "";
    else
        return getName() + "[" + getAbreviation() + "]";
}

bool aClass::isClassString (QString string)
{
    // empty string for empty classes
    if (string == "")
        return true;

    auto list = string.split("[");

    return (list.size()==2 && list[1].endsWith("]"));
}

aClass aClass::fromString(QString string)
{
    // emoty string for empty classes
    if (string == "")
        return aClass{"", ""};

    auto list = string.split("[");

    Q_ASSERT("This is not a valid class string" && (list.size()==2 && list[1].endsWith("]")));
    QString name = list[0];
    QString abreviation = list[1].split("]")[0];

    return aClass(name, abreviation);
}

bool aClass::operator ==(const aClass &b) const
{
    return (name==b.getName()) && (abreviation==b.getAbreviation());
}

bool aClass::operator !=(const aClass &b) const
{
    return (name!=b.getName()) && (abreviation!=b.getAbreviation());
}
