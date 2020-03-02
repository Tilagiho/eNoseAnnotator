#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

#include <QtCore>
#include <QColor>

#include "aclass.h"

class Annotation
{
public:
    Annotation(const QSet<aClass>& classSet = QSet<aClass>{});

    bool isEmpty() const;

    void set(const QList<aClass>& annotationMap);

    const QString toString() const;

    const QString getProbString() const;

    static Annotation fromString(QString string);

    static bool isAnnotationString(QString string);

    const QList<aClass> getClasses() const;

    const QList<QColor> getColor() const;

    bool contains(aClass aclass) const;

    void remove(aClass aclass);

    void changeClass(aClass oldClass, aClass newClass);

    bool operator ==(const Annotation &other) const;

    bool operator !=(const Annotation &other) const;

    aClass::Type getType() const;

private:
    aClass::Type type;
    QSet<aClass> classSet;
};

#endif // CLASSIFICATION_H
