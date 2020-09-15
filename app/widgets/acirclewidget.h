#ifndef ACIRCLEWIDGET_H
#define ACIRCLEWIDGET_H

#include <QWidget>
#include <QtGui>

#include "../classes/annotation.h"

namespace Ui {
class ACircleWidget;
}

class ACircleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ACircleWidget(QWidget *parent = nullptr);
    ~ACircleWidget();

    void set(Annotation annotation);
    void clear();

private:
    Ui::ACircleWidget *ui;
    QBrush brush = Qt::NoBrush;
    QPen pen {Qt::black};

    void paintEvent(QPaintEvent *);
};

#endif // ACIRCLEWIDGET_H
