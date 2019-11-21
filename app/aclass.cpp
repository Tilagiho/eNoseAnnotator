#include "aclass.h"

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

aClass aClass::fromString(QString string)
{
    auto list = string.split("[");
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
