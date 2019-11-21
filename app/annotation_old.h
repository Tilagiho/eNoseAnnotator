#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <array>
#include <QtCore>
#include <QMap>

#include "mvector.h"
#include "measurementdata.h"


class Annotation
{
public:
    Annotation();
    Annotation(QString className, double timestamp, MVector sensorVector, MVector baseLevelVector, QString sensorId, QString comment=QString(""), uint nVectors=1, QList<QString> extraAttributes = QList<QString>(), MeasurementData::MultiMode multiMode=MeasurementData::MultiMode::Average, QString sensorFailureString=QString("0000000000000000000000000000000000000000000000000000000000000000"));

    /*
     * returns string of "class;" + all attribute names seperated by ';', ended by '\n'
     */
    //QString getAttributeString();

    /*
     * returns string of the class + all attributes seperated by ';'
     */
    QString getValueString();

    QString getClassName() const;
    void setClassName(const QString &value);

    QString getTimestamp() const;
    void setTimestamp(double value);

    MVector getSensorVector() const;
    void setSensorVector(const MVector &value);

    QString getSensorId() const;
    void setSensorId(const QString &value);

    QString getComment() const;
    void setComment(const QString &value);

    QList<QString> getExtraAttributes() const;
    void setExtraAttribute(const int pos, const QString &value);
    void addExtraAttribute(const QString &value, int pos);
    void deleteExtraAttribute(const int pos);

    QString getSensorFailureString() const;
    void setSensorFailureString(const QString &value);

    QString toString();

    QString getMultiMode() const;
    void setMultiMode(const MeasurementData::MultiMode &value);

    const QString attribute(int column) const;

private:
    QString className;
    QString timestamp;
    MVector sensorVector;
    MVector baseLevelVector;
    QString sensorId;
    QString comment;
    QList<QString> extraAttributes;
    QString sensorFailureString;
    QString nVectors;
    QString multiMode;

    // attributeList
    // 0:timestamp, 1:className, 2:sensorId, 3:comment, 4:sensorFailureString, 5:nVectors, 6:multiMode
    QList<QString*> attributeList;

};

#endif // ANNOTATION_H
