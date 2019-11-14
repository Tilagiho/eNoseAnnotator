#include "classifierwidget.h"
#include "ui_classifierwidget.h"

ClassifierWidget::ClassifierWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClassifierWidget)
{
    ui->setupUi(this);
}

ClassifierWidget::~ClassifierWidget()
{
    delete ui;
}
