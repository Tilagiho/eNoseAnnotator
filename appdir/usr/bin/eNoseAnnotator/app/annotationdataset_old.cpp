#include "annotationdataset_old.h"

AnnotationDataset::AnnotationDataset(QObject *parent) : QObject(parent),
    annotationData{},
    classList{},
    extraAttributes{}
{

}

AnnotationDataset::~AnnotationDataset()
{}

const QList<Annotation> AnnotationDataset::getAnnotationData() const
{
    return annotationData;
}

void AnnotationDataset::addAnnotation(const Annotation annotation)
{
    annotationData.append(annotation);

    emit annotationDatasetChanged();
}

void AnnotationDataset::addClass (QString className)
{
    Q_ASSERT (!classList.contains(className));

    classList.append(className);
    emit annotationDatasetChanged();
}

void AnnotationDataset::deleteAnnotation(int index)
{
    Q_ASSERT (index < annotationData.size());
    annotationData.removeAt(index);

    emit annotationDatasetChanged();
}

void AnnotationDataset::updateAnnotation(Annotation annotation, int index)
{
    Q_ASSERT(index < annotationData.size());

    annotationData[index] = annotation;
    emit annotationDatasetChanged();
}

void AnnotationDataset::addExtraAttribute(QString name, QString defaultValue)
{
    Q_ASSERT (extraAttributes.contains(name));

    // add new attribute to list of extra attributes
    extraAttributes[name] = defaultValue;

    // update all annotations
    // get index of new attribute
    int index = extraAttributes.keys().indexOf(name);

    for (Annotation annotation : annotationData)
        annotation.addExtraAttribute(defaultValue, index);

    emit annotationDatasetChanged();
}

void AnnotationDataset::renameAttribute(QString oldName, QString newName)
{
    extraAttributes[newName] = extraAttributes[oldName];
    extraAttributes.remove(oldName);

    emit annotationDatasetChanged();
}

void AnnotationDataset::deleteExtraAttribute(QString name)
{
    // remove value of attribute name from all annotations
    for (auto annotation : annotationData)
        annotation.deleteExtraAttribute(extraAttributes.keys().indexOf(name));

    // remove attribute name from attribute map
    extraAttributes.remove(name);

    emit annotationDatasetChanged();
}

void AnnotationDataset::changeDefaultValue(QString attribute, QString defaultValue)
{
    extraAttributes[attribute] = defaultValue;
}

int AnnotationDataset::nAttributes()
{
    // 7 + #extraAttributes:
    // timestamp, className, sensorId, comment, sensorFailureString, nVectors, multiMode, extraAttributes
    return 7 + extraAttributes.size();
}

int AnnotationDataset::nAnnotations()
{
    return annotationData.size();
}

QString AnnotationDataset::attributeName(int index) const
{
    Q_ASSERT (index < 7+extraAttributes.size());

    if (index < 7)
    {
        QList<QString> attributeList {"timestamp", "className", "sensorId", "comment", "sensorFailureString", "nVectors", "multiMode"};
        return attributeList[index];
    } else
        return extraAttributes.keys()[index - 7];
}

const QList<QString> AnnotationDataset::getClasses() const
{
    return classList;
}

void AnnotationDataset::setClasses(const QList<QString> &value)
{
    classList = value;

    emit annotationDatasetChanged();
}

const QMap<QString, QString> AnnotationDataset::getExtraAttributes() const
{
    return extraAttributes;
}

void AnnotationDataset::setExtraAttributes(const QMap<QString, QString> &value)
{
    extraAttributes = value;

    emit annotationDatasetChanged();
}

void AnnotationDataset::clear()
{
    annotationData.clear();
    name.clear();
    classList.clear();
    extraAttributes.clear();

    emit annotationDatasetChanged();
}
