#include "annotationdatasetmodel.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>

AnnotationDatasetModel::AnnotationDatasetModel(QObject* parent):
    annotationData{},
    classList{},
    extraAttributes{},
    QAbstractTableModel(parent)
{
    // init functionalization
    for (int i=0; i<functionalizationArray.size(); i++)
        functionalizationArray[i] = 0;
}


int AnnotationDatasetModel::rowCount(const QModelIndex &parent) const
{
    return annotationData.size();
}

int AnnotationDatasetModel::columnCount(const QModelIndex &parent) const
{
    return fixedAttributeList.size() + extraAttributes.size() + 2*MVector::size;
}

QVariant AnnotationDatasetModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= (fixedAttributeList.size() + extraAttributes.size()))
           return QVariant();

    // get attribute column of annotation at row
    if (role == Qt::DisplayRole)
    {
        Annotation annotation = annotationData[index.row()];
        QString value = annotation.attribute(index.column());
        return value;
    }
    else
        return QVariant();
}

QVariant AnnotationDatasetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
            return attributeName(section);
        else
            return QVariant();
    } else  // vertical
    {
        if (role == Qt::DisplayRole)
            return section;
        else
            return QVariant();
    }
}

void AnnotationDatasetModel::addAnnotation(const Annotation annotation)
{
    Q_ASSERT ("Error: addAnnotation: Uncompatible annotation" && annotation.getExtraAttributes().size() == extraAttributes.size());
    beginInsertRows(QModelIndex(), nAnnotations(), nAnnotations()); // keep QAbstractTableModel in sync
    annotationData.append(annotation);
    endInsertRows();    // keep QAbstractTableModel in sync

    dataChanged = true;
    emit annotationDatasetChanged();
}

void AnnotationDatasetModel::addClass (QString className)
{
    Q_ASSERT (!classList.contains(className));

    classList.append(className);

    dataChanged = true;
    emit annotationDatasetChanged();
}

void AnnotationDatasetModel::deleteAnnotation(int index)
{
    Q_ASSERT (index < annotationData.size());


    beginRemoveRows(QModelIndex(), index, index);   // keep QAbstractTableModel in sync
    annotationData.removeAt(index);
    endRemoveRows();    //  keep QAbstractTableModel in sync

    dataChanged = true;
    emit annotationDatasetChanged();
}

void AnnotationDatasetModel::updateAnnotation(Annotation annotation, int index)
{
    Q_ASSERT("Error: updateAnnotation: index out of range!" && index < annotationData.size());
    Q_ASSERT("Error: updateAnnotation: annotation not compatible!" && annotation.getExtraAttributes().size() == extraAttributes.size());

    beginResetModel();
    annotationData[index] = annotation;
    endResetModel();

    dataChanged = true;
    emit annotationDatasetChanged();
}

void AnnotationDatasetModel::addExtraAttribute(QString name, QString defaultValue)
{
//    qDebug() << "Add extraAttribute " << name << " with default " << defaultValue << " to " << extraAttributes.keys();
    Q_ASSERT ("Error: addExtraAttribute: New attribute already exists!" && !extraAttributes.contains(name));

    // find out where name will be inserted in extraAttributes
    auto keyList = extraAttributes.keys();
    keyList.append(name);
    keyList.sort();
    int index = keyList.indexOf(name);

    beginInsertColumns(QModelIndex(), index+fixedAttributeList.size(), index+fixedAttributeList.size());    // keep QAbstractTableModel in sync
    // add new attribute to list of extra attributes
    extraAttributes[name] = defaultValue;

    // update all annotations
    for (int i=0; i<annotationData.size(); i++)
        annotationData[i].addExtraAttribute(defaultValue, index);

    endInsertColumns();     // keep QAbstractTableModel in sync

    dataChanged = true;
    emit annotationDatasetChanged();
}

void AnnotationDatasetModel::renameAttribute(QString oldName, QString newName)
{
    Q_ASSERT ("Error: renameAttribute: Old attribute does not exist!" && extraAttributes.contains(oldName));
    Q_ASSERT ("Error: renameAttribute: New attribute already exists!" && !extraAttributes.contains(newName));

    extraAttributes[newName] = extraAttributes[oldName];
    extraAttributes.remove(oldName);

    dataChanged = true;
    emit annotationDatasetChanged();
}

void AnnotationDatasetModel::renameClass(QString oldName, QString newName)
{
    Q_ASSERT ("Error: renameClass: Old class does not exist!" && classList.contains(oldName));
    Q_ASSERT ("Error: renameClass: New class already exists!" && !classList.contains(newName));

    classList[classList.indexOf(oldName)] = newName;

    beginResetModel();

    for (int i=0; i<annotationData.size(); i++)
    {
        if (annotationData[i].getClassName() == oldName)
            annotationData[i].setClassName(newName);
    }
    endResetModel();

    dataChanged = true;
    emit annotationDatasetChanged();
}

void AnnotationDatasetModel::deleteExtraAttribute(QString name)
{
    Q_ASSERT ("Error: deleteExtraAttribute: Attribute does not exist!" && extraAttributes.contains(name));

    int index = extraAttributes.keys().indexOf(name);
    beginRemoveColumns(QModelIndex(), index, index);  // keep QAbstractTableModel in sync

    // remove value of attribute name from all annotations
    for (auto annotation : annotationData)
        annotation.deleteExtraAttribute(index);

    // remove attribute name from attribute map
    extraAttributes.remove(name);

    endRemoveColumns();     // keep QAbstractTableModel in sync

    dataChanged = true;
    emit annotationDatasetChanged();
}

void AnnotationDatasetModel::deleteClass(QString className)
{

    classList.takeAt(classList.indexOf(className));

    beginResetModel();
    for (int i=0; i<annotationData.size(); i++)
        if (annotationData[i].getClassName() == className)
            annotationData[i].setClassName("");
    endResetModel();
}

void AnnotationDatasetModel::changeDefaultValue(QString attribute, QString defaultValue)
{
    Q_ASSERT("Error: changeDefaultValue: attribute does not exist!" && extraAttributes.contains(attribute));
    extraAttributes[attribute] = defaultValue;

    dataChanged = true;
}

int AnnotationDatasetModel::nAttributes()
{
    // #fixedAttributes + #extraAttributes:
    // timestamp, className, sensorId, comment, sensorFailureString, nVectors, multiMode, extraAttributes
    return fixedAttributeList.size() + extraAttributes.size();
}

int AnnotationDatasetModel::nAnnotations()
{
    return annotationData.size();
}

QString AnnotationDatasetModel::attributeName(int index) const
{
    Q_ASSERT ("Error: attributeName: index out of range!" && index < fixedAttributeList.size()+extraAttributes.size()+2*MVector::size);

    if (index < fixedAttributeList.size())
        return fixedAttributeList[index];
    else if (index < fixedAttributeList.size() + extraAttributes.size())
        return extraAttributes.keys()[index - fixedAttributeList.size()];
    else if (index < fixedAttributeList.size() + extraAttributes.size() + MVector::size)
        return "s" + QString::number(index - (fixedAttributeList.size() + extraAttributes.size()) +1);
    else
        return "b" + QString::number(index - (fixedAttributeList.size() + extraAttributes.size() + MVector::size) +1);
}

QString AnnotationDatasetModel::getComment() const
{
    return comment;
}

void AnnotationDatasetModel::setComment(const QString &value)
{
    comment = value;
}

const QList<Annotation> AnnotationDatasetModel::getAnnotationData() const
{
    return annotationData;
}

std::array<int, MVector::size> AnnotationDatasetModel::getFuncArray() const
{
    return functionalizationArray;
}

void AnnotationDatasetModel::setFuncArray(const std::array<int, MVector::size> &value)
{
    functionalizationArray = value;
}

const QList<QString> AnnotationDatasetModel::getClasses() const
{
    return classList;
}

const QMap<QString, QString> AnnotationDatasetModel::getExtraAttributes() const
{
    return extraAttributes;
}

void AnnotationDatasetModel::clear()
{
    annotationData.clear();
    comment.clear();
    classList.clear();
    extraAttributes.clear();

    dataChanged = true;

    emit annotationDatasetChanged();
}

void AnnotationDatasetModel::loadData(QWidget* widget)
{
    bool dataSaved = true;

    // ask to save old data
    if (!annotationData.isEmpty() && dataChanged)
    {
        if (QMessageBox::question(widget, tr("Save annotation dataset"),
            "Do you want to save the current annotation dataset before loading data?\t") == QMessageBox::Ok)
            dataSaved=saveData(widget);
    }
    if (!dataSaved)
        return;

    QString fileName = QFileDialog::getOpenFileName(widget, "Open data file", "", "Data files (*.csv)");

    if (fileName.isEmpty())
    {
        return;
    } else
    {

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::information(widget, "Unable to open file",
                file.errorString());
            return;
        }

        // clear data
        clearData();
        emit annotationDatasetChanged();

        // read data from file
        QTextStream in(&file);
        QString line;
        QRegExp rx("\\;");

        // first line: check for version
        QString firstLine = in.readLine();

        if (!firstLine.startsWith("#annotation data "))
        {
            QMessageBox::warning(widget, "Can not read file", "The selected file is not a annotation file!");
            return;
        }
        version = line.right(line.length()-QString("#annotation data ").length());

        bool readOk = true;
        while (in.readLineInto(&line))
        {
            if (line[0] == '#') // meta info
            {
                if (line.startsWith("#$extra attributes:"))  // extra attributes
                {
                    auto attributeList = line.right(line.length()-QString("#$extra attributes:").length()).split(";");

                    for (auto pair : attributeList)
                    {
                        QString attribute = pair.split(":")[0];
                        QString defaultValue = pair.split(":")[1];
                        addExtraAttribute(attribute, defaultValue);
                    }
                } else if (line.startsWith("#$functionalitization:"))   // functionalitization
                {
                    auto funcList = line.right(line.length()-QString("#$functionalitization:").length()).split(";");

                    for (int i=0; i<MVector::size; i++)
                        functionalizationArray[i] = funcList[i].toInt();
                } else if (line.startsWith("#$classes:"))  // classList
                {
                     auto classes = line.right(line.length()-QString("#$classes:").length()).split(";");
                     for (auto className : classes)
                         addClass(className);
                } else    // comment
                    comment += line.right(line.length()-1) + "\n";
            } else  // data
            {
                QStringList query = line.split(";");

                if (query.size() != fixedAttributeList.size()+extraAttributes.size()+2*MVector::size)
                {
                    QMessageBox::critical(static_cast<QWidget*>(parent()), "Error loading annotation data", "Format is not compatible (#attributes:" + QString::number(query.size()) + ").");
                    readOk = false;
                    break;
                }

                // fixed attributes
                // 0:timestamp, 1:className, 2:sensorId, 3:comment, 4:sensorFailureString, 5:nVectors, 6:multiMode, 7-?:extraAttributes
                double aTimestamp = QDateTime::fromString(query[0], "d.M.yyyy - h:mm:ss").toTime_t();
                QString aClassName = query[1];
                QString aSensorId = query[2];
                QString aComment = query[3];
                QString aSensorFailureString = query[4];
                int aNVectors = query[5].toInt();
                MeasurementData::MultiMode aMultiMode = MeasurementData::qStringToMultiMode(query[6]);

                // get extra attributes
                int offset = fixedAttributeList.size();
                QList<QString> aExtraAttributes;
                for (int i=0; i<extraAttributes.size(); i++)
                    aExtraAttributes.append(query[i+offset]);

                // get vector
                offset = fixedAttributeList.size()+ extraAttributes.size();
                MVector aVector;
                for (int i=0; i<MVector::size; i++)
                    aVector.array[i] = query[i+offset].toDouble(&readOk);

                // get baselevel
                MVector baseLevelVector;
                for (int i=0; i<MVector::size; i++)
                    baseLevelVector.array[i] = query[i+offset+MVector::size].toDouble(&readOk);

                addAnnotation(Annotation (aClassName, aTimestamp, aVector, baseLevelVector, aSensorId, aComment, aNVectors, aExtraAttributes, aMultiMode, aSensorFailureString));
            }

            // catch errors reading files
            if (!readOk)
            {
                QMessageBox::critical(widget, "Error reading file", "Error in line:\n" + line);
                clearData();
                return ;
            }
        }

        dataChanged = false;

        emit annotationDatasetChanged();
    }
}

bool AnnotationDatasetModel::saveData(QWidget* widget)
{
    QString fileName = QFileDialog::getSaveFileName(widget, QString("Save annotation dataset"), QString(""), "Data files (*.csv)");

    if (fileName.isEmpty())
    {
        return false;
    } else
    {

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::information(widget, tr("Unable to open file"),
                file.errorString());
            return false;
        }

        QTextStream out(&file);
        // write info
        // version
        out << "#annotation data " << version << "\n";

        // comment
        if (!comment.isEmpty())
        {
            // go through dataComment line-by-line
            QTextStream commentStream(&comment);
            QString line;

            while (commentStream.readLineInto(&line))
                out << "#" << line << "\n";
        }

        // classes
        out << "#$classes:" << classList.join(";") << "\n";

        // extra attributes
        out << "#$extra attributes:";
        QList<QString> pairList;
        for (QString attribute : extraAttributes.keys())
            pairList << attribute +  ":" + extraAttributes[attribute];
        out << pairList.join(";") << "\n";

        // functionalization
        QStringList values;

        for (int i=0; i<MVector::size; i++)
            values << QString::number(functionalizationArray[i]);
        out << "#$functionalitization:" << values.join(";") << "\n";

        // write data
        // 0:timestamp, 1:className, 2:sensorId, 3:comment, 4:sensorFailureString, 5:nVectors, 6:multiMode, 7-?:extraAttributes, ?-?+63:vector, ?+64-?+127
        for (auto annotation : annotationData)
            out << annotation.getValueString() << "\n";

        dataChanged = false;
    }
}

void AnnotationDatasetModel::clearData()
{
    comment.clear();
    extraAttributes.clear();
    annotationData.clear();
    classList.clear();
    dataChanged = false;
}

