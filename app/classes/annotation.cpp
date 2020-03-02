#include "annotation.h"

Annotation::Annotation(const QSet<aClass>& classSet):
    classSet{classSet}
{
    // assert consistent class type:
    if (classSet.size() > 0)
    {
        auto annotationList = classSet.toList();
        type = annotationList.first().getType();

        for (aClass aclass : annotationList)
            Q_ASSERT("Error: inconsistent aClass types in Annotation!" && aclass.getType() == type);
    }
}

bool Annotation::isEmpty() const
{
    // non-empty class in classList
    // -> annotation not empty
    for (auto aclass : classSet)
        if (!aclass.isEmpty())
            return false;
    return true;
}

aClass::Type Annotation::getType() const
{
    return type;
}

void Annotation::set(const QList<aClass>& annotationList)
{
    // assert consistent class type:
    if (annotationList.size() > 0)
    {
        type = annotationList.first().getType();

        for (aClass aclass : annotationList)
            Q_ASSERT("Error: inconsistent aClass types in Annotation!" && aclass.getType() == type);
    }

    this->classSet = annotationList.toSet();
}

/*!
 * \brief Annotation::fromString returns an Annotation based on \a string.
 * \a string should be in format "annotation_class1,annotation_class2,..." where a annotion_class is defined by either a class abreviation or a class abreviation and a value.
 * \param string
 * \return
 */
Annotation Annotation::fromString(QString string)
{
    QSet<aClass> classes;

    for (QString classString : string.split(','))
        classes << aClass::fromString(classString);

    return Annotation(classes);
}

bool Annotation::isAnnotationString(QString string)
{
    for (QString classString : string.split(','))
        if (!aClass::isClassString(classString))
            return false;

    return true;
}


const QList<aClass> Annotation::getClasses() const
{
    QList<aClass> classList = classSet.toList();
    std::sort(classList.begin(), classList.end());
    std::reverse(classList.begin(), classList.end());
    return classList;
}

/*!
 * \brief Annotation::toString
 * Format: class_string1,class_string2,... where class_strings are either of format 'class abreviation' or 'class abreviation:value'.
 * \return
 */
const QString Annotation::toString() const
{
    QList<QString> classStrings;

    for (aClass aclass : getClasses())
        classStrings << aclass.toString();

    return classStrings.join(',');
}

const QString Annotation::getProbString() const
{
   if (getType() != aClass::Type::NUMERIC)
       return toString().split(",").join("\n");

   QList<QString> classStrings;

    for (aClass aclass : getClasses())
    {
        if (aclass.getValue() > 0.001)
            classStrings << aclass.getName() + ": " + QString::number(100*aclass.getValue(), 'f', 1) + "%\n";
    }

    return classStrings.join('\n');
}

/*!
 * \brief Annotation::contains \a aclass is contained in an Annotation, if one class in \a classList matches its name regardless of the class type.
 * \param aclass
 * \return
 */
bool Annotation::contains(aClass aclass) const
{
    for (aClass memberClass : classSet)
    {
        if (aclass.getName() == memberClass.getName())
            return true;
    }
    return false;
}

/*!
 * \brief Annotation::remove \a removes the class with same name as a\a aclass from \a classList.
 * \param aclass
 */
void Annotation::remove(aClass aclass)
{
    for (aClass memberClass : classSet)
    {
        if (aclass.getName() == memberClass.getName())
        {
            classSet.remove(memberClass);
            return;
        }
    }
}

/*!
 * \brief Annotation::changeClass replaces class with name of \a oldClass with name & abreviation of \a newClass. The class value remains unchanged.
 * \param oldClass
 * \param newClass
 */
void Annotation::changeClass(aClass oldClass, aClass newClass)
{
    if (classSet.contains(oldClass))
    {
        classSet.remove(oldClass);
        classSet.insert(newClass);
    }
}


bool Annotation::operator ==(const Annotation &other) const
{
    return classSet == other.getClasses().toSet();
}

bool Annotation::operator !=(const Annotation &other) const
{
    return !(*this == other);
}
