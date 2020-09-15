#include "aclass.h"

#include <QtCore>

#include "enosecolor.h"

/*!
  \var name
  The name of the annotation class.
  Defined to be a combination of any letters, ' ', '-' and '_'.
*/

/*!
  \var abreviation
  The abreviation of the annotation class. Should be kept short & related to the class name.
  Defined to be a combination of any letters.
*/

/*!
 * Stores all known classes. Keys are abreviations, names are values.
*/
static QMap<QString, QString> staticClassMap;

/*!
  \var annotation_regex is used check & extract class strings contataining annotation information about annotation classes.
  \see abreviation, name, getClassTypeName(), getClassType(), name_regex, abr_regex,
*/
static const QRegularExpression numeric_annotation_regex {"^(?<name>.*):(?<value>.*)$"};


/*!
  * \var name_regex defines the format of class names.
  * Class names can be the combination of any letters and the characters ' ', '-' & '_'.
*/
static const QRegularExpression class_name_regex {"^[\\w \\-_]+$"};

/*!
  * \var name_regex defines the format of class names.
  * Class names can be the combination of any letters and the characters ' ', '-' & '_'.
*/
static const QRegularExpression old_class_name_regex {"^(?<name>[\\w -_]+)\\[(?<abreviation>\\w+)\\]$"};

QSet<aClass> aClass::staticClassSet {};


aClass::aClass(QString className, double value):
    name{className},
    value{value}
{}

/*!
 * \brief aClass::aClass copies name & value from \a aclass
 * \param aclass
 */
aClass::aClass(const aClass& aclass)
{
    name = aclass.getName();
    value = aclass.getValue();
}

bool aClass::isEmpty() const
{
    return (name == "");
}

QString aClass::getName() const
{
    return name;
}

/*!
 * \brief aClass::toString returns a QString containing the class abreviaiton and its value (if it was set)
 * The QString returned by toString() is used for annotating single vectors.
 * A QString containing the whole class information is created by getInfoString
 * \return
 */
QString aClass::toString() const
{
    if (isEmpty())
        return "";
    else if (getType() == Type::CLASS_ONLY)
        return getName();
    else if (getType() == Type::NUMERIC)
        return getName() + ":" + QString::number(getValue());
    else
        Q_ASSERT("Unknown type!" && false);
}

double aClass::getValue() const
{
    return value;
}

bool aClass::isClassString (QString string)
{
    // empty string for empty classes
    if (string == "")
        return true;

    // old format:
    // name[abreviation]
    auto match = old_class_name_regex.match(string);

    if (match.hasMatch())
    {
        auto name_match = class_name_regex.match(match.captured("name"));
        auto abr_match = class_name_regex.match(match.captured("abreviation"));

        if (name_match.hasMatch() && abr_match.hasMatch())
            return true;
    }

    // current format:
    // class only -> "<name>"
    match = class_name_regex.match(string);

    if (match.hasMatch())
        return true;

    // numeric -> "<name>:<value>"
    match = numeric_annotation_regex.match(string);
    if (match.hasMatch())
    {
        QString name = match.captured("name");
        QString value = match.captured("value");

        // old version
        if (name.contains('['))
            name = name.split('[')[0];

        auto name_match = class_name_regex.match(name);

        bool valueIsDouble;
        value.toDouble(&valueIsDouble);

        if (name_match.hasMatch() && valueIsDouble)
            return true;
    }

    // no class string
    return false;
}

aClass aClass::fromString(QString string)
{
    QString name;

    // empty string for empty classes
    if (string == "")
        return aClass(QString(""), -1.0);

    // old version:
    // name[abreviation]
    auto match = old_class_name_regex.match(string);

    if (match.hasMatch())
    {
        auto name_match = class_name_regex.match(match.captured("name"));
        auto abr_match = class_name_regex.match(match.captured("abreviation"));

        if (name_match.hasMatch() && abr_match.hasMatch())
            return aClass(match.captured("name"), -1.0);
    }

    // current version meta info:
    // 1st case: class only -> "name"
    match = class_name_regex.match(string);

    if (match.hasMatch())
    {
        name = match.captured();

        return aClass(name, -1.0);
    }
    // 2nd case: numeric -> "name:value"
    match = numeric_annotation_regex.match(string);

    if (match.hasMatch())
    {
        name = match.captured("name");

        // old version
        if (name.contains('['))
            name = name.split('[')[0];

        auto name_match = class_name_regex.match(name);

        bool valueIsDouble;
        double value = match.captured("value").toDouble(&valueIsDouble);

        if (name_match.hasMatch() && valueIsDouble)
            return aClass(name, value);

    }

    // not a class string
    Q_ASSERT(false && "This is not a valid class string");
}

bool aClass::operator ==(const aClass &b) const
{
    return (name==b.getName() &&  qFuzzyCompare(2.0+value, 2.0+b.getValue()));  // adding 2.0, because qFuzzyCompare cannot handle 0.0 as input
}

bool aClass::operator !=(const aClass &b) const
{
    return !(*this == b);
}

aClass::Type aClass::getType() const
{
    if (value < 0.0)
        return Type::CLASS_ONLY;
    else
        return Type::NUMERIC;
}

/*!
 * \brief aClass::setType sets type of class.
 * Warning: Changing aClass::Type::NUMERIC to aClass::Type::CLASS_ONLY deletes \a value. aClass::Type::CLASS_ONLY to aClass::Type::NUMERIC results in value=1.0.
 * \param type
 */
void aClass::setType(Type type)
{
    // type is not new:
    // -> do nothing
    if (type == getType())
        return;
    // class only:
    else if (type == Type::CLASS_ONLY)
        value = -1.0;
    // numeric:
    else if (type == Type::NUMERIC)
        value = 1.0;
    // unknown
    else
        Q_ASSERT(false && "Unknown annotation type!");
}

void aClass::setValue(double value)
{
    value = value;
}

void aClass::setName(const QString &value)
{
    name = value;
}

QString aClass::getClassTypeName(Type type)
{
    if (type == Type::CLASS_ONLY)
        return QString("class_only");
    else if (type == Type::NUMERIC)
        return  QString("numeric");
    else
        Q_ASSERT (false && "enum case not implemented!");
}

aClass::Type aClass::getClassType(QString typeName)
{
    if (typeName == "class_only")
        return Type::CLASS_ONLY;
    else if (typeName == "numeric")
        return Type::NUMERIC;
    else
        Q_ASSERT (false && "enum case not implemented!");
}

QColor aClass::getColor(aClass aclass)
{
    Q_ASSERT("Class is unknown!" && staticClassSet.contains(aClass(aclass.getName())));
    // get number of classes known & position of aclass
    int n_classes = staticClassSet.size();
    int i_aclass = staticClassSet.toList().indexOf(aClass(aclass.getName(), -1.0));

    return ENoseColor::getClassColor(i_aclass, n_classes);
}

/*!
 * \brief aClass::operator < sorts aClass objects by their \a value, if values are equal they are sorted by their name.
 * \param other
 * \return
 */
bool aClass::operator<(const aClass &other) const
{
    if (!qFuzzyCompare(this->getValue()+2.0, other.getValue()+2.0)) // add 2.0, because qFuzzyCompare cannot handle inputs equal to 0.0 & values are in range [-1.0;+infinite]
        return this->getValue() < other.getValue();

    return this->getName() < other.getName();
}

inline bool aClass::operator> (const aClass &other) const { return other < *this; }
inline bool aClass::operator<=(const aClass &other) const { return !(*this > other); }
inline bool aClass::operator>=(const aClass &other) const { return !(*this < other); }
