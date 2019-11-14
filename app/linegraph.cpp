#include "linegraph.h"
#include "ui_linegraph.h"
#include "sensorcolor.h"

#include <math.h>


lineGraph::lineGraph(QWidget *parent, uint startTime) :
    QWidget(parent),
    ui(new Ui::lineGraph)
{
    ui->setupUi(this);

    // init start timestamp
    startTimestamp = startTime;

    setupGraph();

    // connections:
    // update y-scale when changing x-range
    connect(ui->chart->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(replot()));
    // set selectionRectMode when mouse is pressed
    connect(ui->chart, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePressed(QMouseEvent*)));
    // process data selection
    connect(ui->chart, SIGNAL(selectionChangedByUser()), this, SLOT(dataSelected()));

    // test: add measurement
    QVector<MVector> measurementVector;
    QVector<double> timestampVector;
}

lineGraph::~lineGraph()
{
    delete ui;
}

void lineGraph::setupGraph()
{
    // graph is dragable & zoomable in x direction
    ui->chart->setInteraction(QCP::iRangeDrag, true);
    ui->chart->axisRect()->setRangeDrag(Qt::Horizontal);

    ui->chart->setInteraction(QCP::iRangeZoom, true);
    ui->chart->axisRect()->setRangeZoom(Qt::Horizontal);

    // make graphs selectable
    ui->chart->setInteraction(QCP::iSelectPlottables);

    // init 64 graphs
    for (uint i=0; i<MVector::size; i++)
    {
        ui->chart->addGraph();

        // style of plotted lines
        QColor color = SensorColor::getColor(i);
        ui->chart->graph(i)->setLineStyle(QCPGraph::lsLine);
        ui->chart->graph(i)->setPen(QPen(color));
        ui->chart->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 3));

        // make graphs selectable
         ui->chart->graph(i)->setSelectable(QCP::stDataRange);
    }


    // configure bottom axis to show time:
    QSharedPointer<QCPAxisTickerTime> dateTicker(new QCPAxisTickerTime);
    dateTicker->setTimeFormat("%h:%m:%s");
    ui->chart->xAxis->setTicker(dateTicker);

    // set a more compact font size for bottom and left axis tick labels:
    ui->chart->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
    ui->chart->yAxis->setTickLabelFont(QFont(QFont().family(), 8));

    // set dark background gradient:
    QLinearGradient gradient(0, 0, 0, 400);
    gradient.setColorAt(0, QColor(90, 90, 90));
    gradient.setColorAt(0.38, QColor(105, 105, 105));
    gradient.setColorAt(1, QColor(70, 70, 70));
    ui->chart->setBackground(QBrush(gradient));

    // set ticker colors
    ui->chart->xAxis->setTickLabelColor(Qt::white);
    ui->chart->xAxis->setLabelColor(Qt::white);
    ui->chart->xAxis->setBasePen(QPen(Qt::white));
    ui->chart->xAxis->setTickPen(QPen(Qt::white));
    ui->chart->xAxis->setSubTickPen(QPen(Qt::white));

    ui->chart->yAxis->setTickLabelColor(Qt::white);
    ui->chart->yAxis->setLabelColor(Qt::white);
    ui->chart->yAxis->setBasePen(QPen(Qt::white));
    ui->chart->yAxis->setTickPen(QPen(Qt::white));
    ui->chart->yAxis->setSubTickPen(QPen(Qt::white));


    // set axis ranges to show all data:
    ui->chart->xAxis->setRange(-1, defaultXWidth);
    ui->chart->yAxis->rescale(true);
}

void lineGraph::setStartTimestamp(uint timestamp)
{
    startTimestamp = timestamp;
}

void lineGraph::setXAxis(double x1, double x2)
{
    ui->chart->xAxis->setRange(x1, x2);
}

void lineGraph::setLogXAxis(bool logOn)
{
    if (logOn)
    {
        ui->chart->yAxis->setScaleType(QCPAxis::ScaleType::stLogarithmic);
        QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
        ui->chart->yAxis->setTicker(logTicker);

    }
    else
    {
        ui->chart->yAxis->setScaleType(QCPAxis::ScaleType::stLinear);
        QSharedPointer<QCPAxisTicker> linTicker(new QCPAxisTicker);
        ui->chart->yAxis->setTicker(linTicker);
    }
}

void lineGraph::setMaxVal(double val)
{
    maxVal = val;
}

void lineGraph::replot(uint timestamp)
{   
    //!set new y-range
    double y_lower = 1000;
    double y_upper = 0;

    //!get current xaxis.range
    double x_axis_range_lower = ui->chart->xAxis->range().lower;
    double x_axis_range_upper = ui->chart->xAxis->range().upper;

    //iterate through graph(i) data keys and values

    for (int i = 0; i < ui->chart->graphCount(); ++i) {
        QCPGraphDataContainer::const_iterator it = ui->chart->graph(i)->data()->constBegin();
        QCPGraphDataContainer::const_iterator itEnd = ui->chart->graph(i)->data()->constEnd();
        while (it != itEnd)
        {
          if (it->key >= x_axis_range_lower && it->key <= x_axis_range_upper ){
              if (it->value < y_lower ){
                  y_lower = it->value;
              }
              if (it->value > y_upper ){
                  y_upper = it->value;
              }
          }
          ++it;
        }
    }
    // log plot for big y-intervals
    if (y_upper-y_lower > 1000.0)
    {
        setLogXAxis(true);
        y_upper *= 1.1;
        y_lower *= 0.9;
    }
    // normal plot for medium y-intervals
    else if (y_lower > 100.0)
    {
        setLogXAxis(false);
        y_upper *= 1.2;
        y_lower *= 0.8;
    }
    // plot around 0 for small y-intervals
    else
    {
        // check for minimal y range
        setLogXAxis(false);
        if (y_upper < yMin)
            y_upper = yMin;
        if (y_lower > -yMin)
            y_lower = -yMin;

        y_upper *= 1.2;
        y_lower *= 1.2;
    }

    //set y-range
    ui->chart->yAxis->setRange(y_lower, y_upper);

    // move x-range
    if (timestamp != 0 && timestamp >= x_axis_range_upper+startTimestamp-4 && timestamp <= x_axis_range_upper+startTimestamp)
        ui->chart->xAxis->setRange(x_axis_range_lower+2, x_axis_range_upper+2);

    ui->chart->replot();
}

void lineGraph::mousePressed(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
        {
            // reset selection flag
            selectionFlag = false;

            // enable rectangle selection
            ui->chart->setSelectionRectMode(QCP::srmSelect);

        }
        else
            // disable rectangle selection
            ui->chart->setSelectionRectMode(QCP::srmNone);
    }
}

void lineGraph::dataSelected()
{
    // get selection
    QCPDataSelection selection;
    for (int i=0; i<MVector::size; i++)
        if (ui->chart->graph(i)->selected())
            selection = ui->chart->graph(0)->selection();

    // find x range of selection rectangle
    QCPRange range = ui->chart->selectionRect()->range(ui->chart->xAxis);

    ui->chart->deselectAll();

//    auto start = ui->chart->graph(0)->data()->findBegin(range.lower, false);
//    auto end = ui->chart->graph(0)->data()->findEnd(range.upper, false);

//    // create selection base on range found
//    QCPDataRange dataRange (static_cast<int>(start->key), static_cast<int>(end->key));
//    QCPDataSelection selection (dataRange);

//    if (selection.isEmpty() || selection.dataRange(0).end() <= selection.dataRange(0).begin())
//    {
//        dataSelection.clear();
//        emit selectionCleared();
//        qDebug() << "selection cleared:"  << QString::number(getSelection()[0], 'g', 10) << ", " << QString::number(getSelection()[1], 'g', 10);
//    }
    int lower = ceil(range.lower);
    int upper = floor(range.upper);

    qDebug() << "Selection Rect: " << lower << ", " << upper;

    if (upper<lower)
    {
        dataSelection.clear();
        emit selectionCleared();
    }
    else
    {
        qDebug() << "Got selection: " << selection;

//        QCPDataRange dataRange (lower/2, upper/2+1);
//        QCPDataSelection selection (dataRange);

        // select all graphs in range of selection
        for (int i=0; i < ui->chart->graphCount(); i++)
        {
            ui->chart->graph(i)->setSelection(selection);
        }

        // save selection and emit signal for changed selection
        dataSelection = selection;
        emit selectionChanged(lower+startTimestamp, upper+startTimestamp);
    }
}

void lineGraph::clearGraph()
{
    for (int i=0; i<MVector::size; i++)
    {
        ui->chart->graph(i)->data()->clear();
    }
    ui->chart->replot();
}

void lineGraph::addMeasurement(MVector measurement, uint timestamp, bool rescale)
{
    if (ui->chart->graph(0)->data()->isEmpty())
        setStartTimestamp(timestamp);

    for (int i=0; i<MVector::size; i++)
    {
        QCPGraphData graphData;
        if (!useLimits || (measurement.array[i] > minVal && measurement.array[i] < maxVal))
            ui->chart->graph(i)->addData(round(timestamp-startTimestamp), measurement.array[i]);
        else
            emit sensorFailure(i);
    }

    if (rescale)
        replot(timestamp);
}

void lineGraph::setData(QMap<uint, MVector> map)
{
    clearGraph();

    if (map.isEmpty())
        return;

    setStartTimestamp(map.firstKey());

    for (auto timestamp : map.keys())
        addMeasurement(map[timestamp], timestamp);

    setXAxis(-1, defaultXWidth);
    replot();
}

double lineGraph::getMinVal() const
{
    return minVal;
}

void lineGraph::setMinVal(double value)
{
    minVal = value;
}

double lineGraph::getMaxVal() const
{
    return maxVal;
}

std::array<uint, 2> lineGraph::getSelection()
{
    // get data range of selected data
    QCPDataRange dataRange = dataSelection.dataRange();

    double start = ui->chart->graph(0)->data()->at(dataRange.begin())->key;
    double end = ui->chart->graph(0)->data()->at(dataRange.end())->key;

    std::array<uint, 2> selectionArray;
    selectionArray[0] = start+startTimestamp-1;
    selectionArray[1] = end+startTimestamp-2;

    return selectionArray;
}
