#include "classifierwidget.h"
#include "ui_classifierwidget.h"

#include <QtCore>
#include <QtGui>

ClassifierWidget::ClassifierWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClassifierWidget)
{
    ui->setupUi(this);

    ui->widget->clear();
    ui->widget->setVisible(false);
}

ClassifierWidget::~ClassifierWidget()
{
    delete ui;
}

void ClassifierWidget::setAnnotation(Annotation annotation)
{
    // first class in classList has highest value
    // -> detected class
    aClass detectedClass = annotation.getClasses().first();
    ui->widget->set(detectedClass.getName());

    QString annotationString = annotation.toString();
    ui->widget->setToolTip(annotationString.split(",").join("\n"));

    setHidden(false);
}

void ClassifierWidget::setClassifier(QString name, QStringList classNames,bool isInputAbsolute)
{
    ui->nameLabel->setText(name);
    ui->classLabel->setText(classNames.join(", "));

    if (isInputAbsolute)
        ui->input_type_label->setText("absolute");
    else
        ui->input_type_label->setText("relative");

    setHidden(false);

}

void ClassifierWidget::setLiveClassification(bool isLive)
{
    isLive = isLive;
}

void ClassifierWidget::setInfoString(QString string)
{
    ui->infoLabel->setHidden(false);
    ui->infoLabel->setText(string);
}


void ClassifierWidget::clear()
{
    ui->nameLabel->setText("No classifier loaded.");

    setHidden(true);

    isSelectionAnnotation = false;
}

void ClassifierWidget::clearAnnotation()
{
    ui->widget->clear();
    ui->widget->setHidden(true);

    ui->infoLabel->setHidden(true);

    isSelectionAnnotation = false;
}

bool ClassifierWidget::getIsLive() const
{
    return isLive;
}

void ClassifierWidget::setHidden(bool hidden)
{
    ui->label_1->setHidden(hidden);
    ui->label_2->setHidden(hidden);

    ui->classLabel->setHidden(hidden);
    ui->input_type_label->setHidden(hidden);

    ui->infoLabel->setHidden(hidden);

    ui->widget->setHidden(hidden);
}
