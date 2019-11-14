#ifndef ANNOTATIONDATA_H
#define ANNOTATIONDATA_H

#include <QObject>
#include "annotation.h"

class AnnotationDataset : public QObject
{
    Q_OBJECT
public:
    explicit AnnotationDataset(QObject *parent = nullptr);
    ~AnnotationDataset();

    const QList<Annotation> getAnnotationData() const;

    const QMap<QString, QString> getExtraAttributes() const;
    void setExtraAttributes(const QMap<QString, QString> &value);

    const QList<QString> getClasses() const;
    void setClasses(const QList<QString> &value);

signals:
    void annotationDatasetChanged();

public slots:
    void addAnnotation (const Annotation annotation);
    void addClass (QString className);
    void updateAnnotation (const Annotation annotation, int index);
    void deleteAnnotation(int number);
    void clear();
    void addExtraAttribute(const QString name, QString defaultValue="");
    void renameAttribute(QString oldName, QString newName);
    void deleteExtraAttribute(const QString name);
    void changeDefaultValue(QString attribute, QString defaultValue);
    int nAnnotations();
    int nAttributes();
    QString attributeName(int index) const;

private:
    QString name;
    QMap<QString, QString> extraAttributes;
    QList<Annotation> annotationData;
    QList<QString> classList;
};

#endif // ANNOTATIONDATA_H
