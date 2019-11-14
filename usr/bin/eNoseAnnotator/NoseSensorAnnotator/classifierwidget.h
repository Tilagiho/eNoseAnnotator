#ifndef CLASSIFIERWIDGET_H
#define CLASSIFIERWIDGET_H

#include <QtCore>
#include <QWidget>

#include "mvector.h"

namespace Ui {
class ClassifierWidget;
}

class ClassifierWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClassifierWidget(QWidget *parent = nullptr);
    ~ClassifierWidget();

private:
    Ui::ClassifierWidget *ui;
};

#endif // CLASSIFIERWIDGET_H
