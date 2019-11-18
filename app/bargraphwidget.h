#ifndef BARGRAPHWIDGET_H
#define BARGRAPHWIDGET_H

#include <QtCore>
#include "mvector.h"

#include "qcustomplot.h"
#include <QWidget>


namespace Ui {
class BarGraphWidget;
}

class BarGraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BarGraphWidget(QWidget *parent = nullptr);
    ~BarGraphWidget();

public slots:
    void setBars(MVector, std::array<bool, MVector::size> sensorFailures);
    void clearBars();

private:
    Ui::BarGraphWidget *ui;
    QVector<QCPBars*> barVector;    // contains 64 bars; one for each sensor in order to set seperate colors for each sensor

    void initGraph();
    void replot();
};

#endif // BARGRAPHWIDGET_H
