#ifndef LINEGRAPH_H
#define LINEGRAPH_H

#include <QtCore>
#include "qcustomplot.h"
#include <QWidget>
#include"mvector.h"

namespace Ui {
class lineGraph;
}

class lineGraph : public QWidget
{
    Q_OBJECT

public:


    explicit lineGraph(QWidget *parent = nullptr, uint startTime = QDateTime::currentDateTime().toTime_t());
    ~lineGraph();
    void clearGraph();
    bool selectionEmpty();
    std::array<uint, 2> getSelection();  // returns start and end timestamp of selection
    void setStartTimestamp(uint timestamp);
    void setXAxis(double x1, double x2);
    void setLogXAxis(bool logOn);
    void setMaxVal(double val);

    double getMaxVal() const;

    double getMinVal() const;
    void setMinVal(double value);

    // flag to determine if minVal and maxVal should be used when plotting data
    bool useLimits = true;


public slots:
    void addMeasurement(MVector measurement, uint timestamp, bool rescale=false);   // add single measurement; rescale y-axis if rescale==true
//    void addMeasurement(QVector<MVector> measurements, QVector<uint> timestamps);   // add multiple measurements
//    void addMeasurement(QMap<uint, MVector>);
    void setData(QMap<uint, MVector> map);

signals:
    void selectionChanged(int, int);
    void selectionCleared();
    void sensorFailure(int i);

private:
    Ui::lineGraph *ui;
    const int defaultXWidth = 30; // defines default range of xAxis: (-1; defaultXWidth)
    const double yMin = 2.0;    // defines minimum range of yAxis: (-yMin;yMin)

    uint startTimestamp; // timestamp for start of graph
    QCPDataSelection dataSelection; // holds current data selection
    bool selectionFlag = false;     // used to avoid looping behaviour when selecting data

    // interval in which data is plotted: data > maxVal or < minVal is ignored
    // can be ignored with useLimits
    double maxVal = 90000.0;
    double minVal = 300.0;

    void setupGraph();

private slots:
    void replot(uint timestamp=0);
    void mousePressed(QMouseEvent*);
    void dataSelected();
};

#endif // LINEGRAPH_H
