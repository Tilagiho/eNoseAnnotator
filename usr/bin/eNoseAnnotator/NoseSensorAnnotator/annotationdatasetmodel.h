#ifndef ANNOTATIONDATASETMODEL_H
#define ANNOTATIONDATASETMODEL_H

#include <QtCore>
#include <QObject>
#include <QAbstractTableModel>
#include "mvector.h"
#include "annotation.h"

class AnnotationDatasetModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    AnnotationDatasetModel(QObject *parent=nullptr);

    // implementing QAbstractTableModel for read-only usage
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;


    // AnnotationData
    const QList<Annotation> getAnnotationData() const;

    const QMap<QString, QString> getExtraAttributes() const;

    const QList<QString> getClasses() const;

    void loadData(QWidget*);

    bool saveData(QWidget*);

    void clearData();

    std::array<int, MVector::size> getFuncArray() const;
    void setFuncArray(const std::array<int, MVector::size> &value);

    QString getComment() const;


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
    void renameClass(QString oldName, QString newName);
    void deleteExtraAttribute(const QString name);
    void deleteClass(const QString name);
    void changeDefaultValue(QString attribute, QString defaultValue);
    void setComment(const QString &value);
    int nAnnotations();
    int nAttributes();
    QString attributeName(int index) const;

private:
    QString comment;
    QMap<QString, QString> extraAttributes;
    QList<Annotation> annotationData;
    QList<QString> classList;
    QList<QString> fixedAttributeList {"timestamp", "className", "sensorId", "comment", "sensorFailureString", "nVectors", "multiMode"};
    std::array<int, MVector::size> functionalizationArray;
    bool dataChanged = false;
    QString version = "v0.1";
};

#endif // ANNOTATIONDATASETMODEL_H
