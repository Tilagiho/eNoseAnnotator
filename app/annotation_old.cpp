#include "annotation_old.h"
#include "mvector.h"

#include <QDateTime>

Annotation::Annotation()
{}

Annotation::Annotation(QString className, double timestamp, MVector sensorVector, MVector baseLevelVector, QString sensorId, QString comment, uint nVectors, QList<QString> extraAttributes, MeasurementData::MultiMode multiMode , QString sensorFailureString):
    className(className),
    sensorVector(sensorVector),
    baseLevelVector(baseLevelVector),
    sensorId(sensorId),
    comment(comment),
    extraAttributes(extraAttributes),
    sensorFailureString(sensorFailureString)
{
    setTimestamp(timestamp);
    setMultiMode(multiMode);
    this->nVectors = QString::number(nVectors);


    attributeList << &this->timestamp << &this->className << &this->sensorId << &this->comment << &this->sensorFailureString << &this->nVectors << &this->multiMode;
}

QString Annotation::getClassName() const
{
    return className;
}

void Annotation::setClassName(const QString &value)
{
    className = value;
}

QString Annotation::getTimestamp() const
{
    return timestamp;
}

void Annotation::setTimestamp(double value)
{
    Q_ASSERT (value > 0);
    timestamp = QDateTime::fromTime_t(static_cast<uint> (value)).toString("d.M.yyyy - h:mm:ss");
}

MVector Annotation::getSensorVector() const
{
    return sensorVector;
}

void Annotation::setSensorVector(const MVector &value)
{
    sensorVector = value;
}

QString Annotation::getSensorId() const
{
    return sensorId;
}

void Annotation::setSensorId(const QString &value)
{
    sensorId = value;
}

QString Annotation::getComment() const
{
    return comment;
}

void Annotation::setComment(const QString &value)
{
    comment = value;
}

QList<QString> Annotation::getExtraAttributes() const
{
    return extraAttributes;
}

void Annotation::setExtraAttribute(const int pos, const QString &value)
{
    Q_ASSERT (extraAttributes.size() > pos);
    extraAttributes[pos] = value;
}

void Annotation::addExtraAttribute( const QString &value, int pos)
{
    Q_ASSERT (pos >= 0 && pos <= extraAttributes.size() && "Insert position should be in interval 0 <= pos <= extraAttibute.size() ");
    extraAttributes.insert(pos, value);
}

void Annotation::deleteExtraAttribute(const int pos)
{
    Q_ASSERT (extraAttributes.size() > pos);
    extraAttributes.removeAt(pos);
}

QString Annotation::getSensorFailureString() const
{
    return sensorFailureString;
}

void Annotation::setSensorFailureString(const QString &value)
{
    Q_ASSERT(value.length() == 64);
    sensorFailureString = value;
}

QString Annotation::getMultiMode() const
{
    return multiMode;
}

void Annotation::setMultiMode(const MeasurementData::MultiMode &value)
{
    multiMode = MeasurementData::multiModeToQString(value);
}

const QString Annotation::attribute(int column) const
{
    Q_ASSERT(column < (7 + extraAttributes.size() + 2*MVector::size));
    Q_ASSERT(column >= 0);

    QString value;

    // 0:timestamp, 1:className, 2:sensorId, 3:comment, 4:sensorFailureString, 5:nVectors, 6:multiMode, 7-?:extraAttributes
//    if (column < 7)
//        value = *(attributeList[column]);
//    else
//        value = extraAttributes[column-7];

    switch (column)
    {
        // timestamp
        case 0:
        {
            value = timestamp;
            break;
        }
        // className
        case 1:
        {
            value = className;
            break;
        }
        // sensorId
        case 2:
        {
            value = sensorId;
            break;
        }
        // comment
        case 3:
        {
            value = comment;
            break;
        }
        // sensorFailureString
        case 4:
        {
            value = sensorFailureString;
            break;
        }
        // nVectors
        case 5:
        {
            value = nVectors;
            break;
        }
        // multiMode
        case 6:
        {
            value = multiMode;
            break;
        }
        // extraAttributes, vector, baselevel
        default:
        {
            // extraAttributes
            if (column-7 < extraAttributes.size())
                value = extraAttributes[column-7];
            // vector
            else if (column-7-extraAttributes.size() < MVector::size)
                value = QString::number(sensorVector.array[column-7-extraAttributes.size()]);
            // baselevel
            else
                value = QString::number(baseLevelVector.array[column-7-extraAttributes.size()-MVector::size]);
        }
    }

    return value;
}


/*
QString Annotation::getAttributeString()
{
    // default attributes
    QString attributeString("class;timestamp;sensorId;nVectors;");

    // extra attributes
    for (QString attribute : extraAttributes.keys())
        attributeString += attribute + ";";

    // sensor attributes
    for (int i=0; i<MVector::size; i++)
        attributeString += "s" + QString::number(i) + ";";

    // comment
    attributeString += "comment";

    // sensor failure bits
    for (int i=0; i<MVector::size; i++)
        attributeString += ";fb" + QString::number(i);
}
*/

QString Annotation::getValueString()
{
    QString valueString("");
    // 0:timestamp, 1:className, 2:sensorId, 3:comment, 4:sensorFailureString, 5:nVectors, 6:multiMode, 7-?:extraAttributes, ?-?+63:vector, ?+64-?+123:baselevel
    QList<QString> attributeList;
    // default attributes
    // class;timestamp;sensorId;nVectors;
    attributeList << timestamp << className << sensorId << comment << sensorFailureString << nVectors << multiMode ;


    // extra attributes
    for (QString attribute : extraAttributes)
        attributeList << attribute;

    // sensor attributes
    for (int i=0; i<MVector::size; i++)
        attributeList << QString::number(sensorVector.array[i]);

    // baseLevel
    for (int i=0; i<MVector::size; i++)
        attributeList << QString::number(baseLevelVector.array[i]);

    valueString = attributeList.join(";");

    return valueString;
}

