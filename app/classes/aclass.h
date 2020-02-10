#ifndef ACLASS_H
#define ACLASS_H

#include <QtCore>

/*!
 * \class aClass
 * contains information of one classification
 */
class aClass
{
public:
    enum class Type {
        CLASS_ONLY,  // the annotation consists of * aClasses _without_ numeric values
        NUMERIC,     // the annotation consists of * aClasses _with_ numeric values
        UNSPECIFIED  // not yet set
    };

    static QString getClassTypeName(Type);
    static Type getClassType(QString);

    static QSet<aClass> staticClassSet;   // contains all classes added in CLASS_ONLY form
    static QColor getColor(aClass);

    aClass(QString name,  double value = -1.0);
    aClass(const aClass& aclass);

    bool isEmpty() const;

    void setClass(QString name, double value = -1.0);

    QString getName() const;

    double getValue() const;

    QString toString() const;

    static bool isClassString (QString string);

    static aClass fromString(QString string);

    bool operator ==(const aClass &b) const;

    bool operator !=(const aClass &b) const;

    bool operator< (const aClass &other) const;

    bool operator> (const aClass &other) const;

    bool operator<= (const aClass &other) const;

    bool operator>= (const aClass &other) const;


    Type getType() const;

    void setType(Type type);

    void setValue(double value);

    void setName(const QString &value);

private:
    QString name;
    double value = -1.0;
};

inline uint qHash(const aClass &key, uint seed)
{
    return qHash(key.getName(), seed);
}

#endif // ACLASS_H
