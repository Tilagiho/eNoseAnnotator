#ifndef CLASSIFIERWIDGET_H
#define CLASSIFIERWIDGET_H

#include <QtCore>
#include <QWidget>

#include "../classes/mvector.h"

namespace Ui {
class ClassifierWidget;
}

class ClassifierWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClassifierWidget(QWidget *parent = nullptr);
    ~ClassifierWidget();

    bool getIsLive() const;

    bool isSelectionAnnotation = false;

public slots:
    void setAnnotation (Annotation annotation);
    void setClassifier (QString name, QStringList classNames,bool isInputAbsolute, QString inputType="Vector");
    void setLiveClassification (bool isLive);
    void setInfoString(QString string);
    void clear();
    void clearAnnotation();

private:
    Ui::ClassifierWidget *ui;
    bool isLive = true;

    void setHidden(bool visible);
};

#endif // CLASSIFIERWIDGET_H
