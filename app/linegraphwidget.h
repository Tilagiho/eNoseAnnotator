#ifndef LINEGRAPH_H
#define LINEGRAPH_H

#include <QtCore>
#include "qcustomplot.h"
#include <QWidget>
#include"mvector.h"

namespace Ui {
class LineGraphWidget;
}

class LineGraphWidget : public QWidget
{
    Q_OBJECT

public:


    explicit LineGraphWidget(QWidget *parent = nullptr, uint startTime = QDateTime::currentDateTime().toTime_t());
    ~LineGraphWidget();
    void clearGraph(bool replot = true);

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


    bool getUseLimits() const;
    void setUseLimits(bool value);

public slots:
    void addMeasurement(MVector measurement, uint timestamp, bool rescale=false);   // add single measurement; rescale y-axis if rescale==true
//    void addMeasurement(QVector<MVector> measurements, QVector<uint> timestamps);   // add multiple measurements
//    void addMeasurement(QMap<uint, MVector>);
    void setData(QMap<uint, MVector> map);
    void setSensorFailureFlags(const std::array<bool, MVector::size> sensorFailureFlags);
    void setAutoMoveGraph(bool value);
    void clearSelection();

    /*
     * draws selection and class rectangles
     */
    void labelSelection(QMap<uint, MVector> selectionMap);

signals:
    void selectionChanged(int, int);
    void selectionCleared();
    void sensorFailure(int i);
    void requestRedraw();

private:
    Ui::LineGraphWidget *ui;
    const int defaultXWidth = 30; // defines default range of xAxis: (-1; defaultXWidth)
    const double yMin = 2.5;    // defines minimum range of yAxis: (-yMin;yMin)

    const double labelRatio = 2.0/50.0;

    bool useLimits = true;

    uint startTimestamp; // timestamp for start of graph
    QCPDataSelection dataSelection; // holds current data selection
    bool selectionFlag = false;     // used to avoid looping behaviour when selecting data

    // interval in which data is plotted: data > maxVal or < minVal is ignored
    // can be ignored with useLimits
    double maxVal = 90000.0;
    double minVal = 300.0;

    std::array<bool, MVector::size> sensorFailureFlags;
    bool autoMoveGraph = true;

    QMap<int, QCPItemText *> userDefinedClassLabels;
    QMap<int, QCPItemText *> detectedClassLabels;

    QMap<double, QCPItemText *> joinedUserDefinedClassLabels;
    QMap<double, QCPItemText *> joinedDetectedClassLabels;

    void setupGraph();

    /*
     * returns index of data point where x == key on chart
     * returns -1 if not found
     */
    double getIndex (int key);

private slots:
    void replot(uint timestamp=0);
    void mousePressed(QMouseEvent*);
    void dataSelected();

    /*
     * draws label of user+detected class at top of graph
     */
    void setLabel(int xpos, QString userDefinedBrief, QString detectedBrief);

    /*
     *  goes through userDefinedLabels and detectedLabels in range of x-axis
     *  joins successive labels with matching classes
     */
    void redrawLabels();
};

#endif // LINEGRAPH_H
