#include "acirclewidget.h"
#include "ui_acirclewidget.h"

#include<QPainter>

#include "../classes/classifier_definitions.h"

ACircleWidget::ACircleWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ACircleWidget)
{
    ui->setupUi(this);
}

ACircleWidget::~ACircleWidget()
{
    delete ui;
}

void ACircleWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.setBrush(brush);
    QPoint circlePos = ui->label->pos() - QPoint(40, -10);
    painter.drawEllipse(circlePos,15,15);
}

void ACircleWidget::set(Annotation annotation)
{
    // first class in classlist has highest value
    // -> detected class
    pen = QPen(Qt::black);

    aClass maxLikelyClass(NO_SMELL_STRING, 0.);
    for (aClass aclass : annotation.getClasses())
        if (aclass.getValue() > maxLikelyClass.getValue())
            maxLikelyClass = aClass(aclass);

    // set color of class with highest probability
    brush = QBrush(aClass::getColor(maxLikelyClass));
    ui->label->setText(maxLikelyClass.toString());
    repaint();
}

void ACircleWidget::clear()
{
    brush = Qt::NoBrush;
    pen = Qt::NoPen;
    ui->label->setText("");
    repaint();
}
