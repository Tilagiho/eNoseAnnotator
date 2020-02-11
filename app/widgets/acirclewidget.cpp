#include "acirclewidget.h"
#include "ui_acirclewidget.h"

#include<QPainter>

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

void ACircleWidget::set(aClass detectedClass)
{
    // first class in classlist has highest value
    // -> detected class
    pen = QPen(Qt::black);
    brush = QBrush(aClass::getColor(detectedClass));
    ui->label->setText(detectedClass.toString());
    repaint();
}

void ACircleWidget::clear()
{
    brush = Qt::NoBrush;
    pen = Qt::NoPen;
    ui->label->setText("");
    repaint();
}
