#include "linegraphwidget.h"
#include "ui_linegraphwidget.h"
#include "sensorcolor.h"

#include <math.h>


LineGraphWidget::LineGraphWidget(QWidget *parent, uint startTime) :
    QWidget(parent),
    ui(new Ui::LineGraphWidget)
{
    ui->setupUi(this);

    // init start timestamp
    startTimestamp = startTime;

    // zero init sensorFailureFlags
    for (int i=0; i<MVector::size; i++)
        sensorFailureFlags[i] = false;

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

LineGraphWidget::~LineGraphWidget()
{
    delete ui;
}

void LineGraphWidget::setupGraph()
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
//    QLinearGradient gradient(0, 0, 0, 400);
//    gradient.setColorAt(0, QColor(90, 90, 90));
//    gradient.setColorAt(0.38, QColor(105, 105, 105));
//    gradient.setColorAt(1, QColor(70, 70, 70));
//    ui->chart->setBackground(QBrush(gradient));

    ui->chart->setBackground(QBrush(QColor(255,250,240)));

    // set ticker colors
//    ui->chart->xAxis->setTickLabelColor(Qt::white);
//    ui->chart->xAxis->setLabelColor(Qt::white);
//    ui->chart->xAxis->setBasePen(QPen(Qt::white));
//    ui->chart->xAxis->setTickPen(QPen(Qt::white));
//    ui->chart->xAxis->setSubTickPen(QPen(Qt::white));

//    ui->chart->yAxis->setTickLabelColor(Qt::white);
//    ui->chart->yAxis->setLabelColor(Qt::white);
//    ui->chart->yAxis->setBasePen(QPen(Qt::white));
//    ui->chart->yAxis->setTickPen(QPen(Qt::white));
//    ui->chart->yAxis->setSubTickPen(QPen(Qt::white));

    ui->chart->xAxis->setTickLabelColor(Qt::black);
    ui->chart->xAxis->setLabelColor(Qt::black);
    ui->chart->xAxis->setBasePen(QPen(Qt::black));
    ui->chart->xAxis->setTickPen(QPen(Qt::black));
    ui->chart->xAxis->setSubTickPen(QPen(Qt::black));
    ui->chart->xAxis->setLabel("Time since start of measurement");

    ui->chart->yAxis->setTickLabelColor(Qt::black);
    ui->chart->yAxis->setLabelColor(Qt::black);
    ui->chart->yAxis->setBasePen(QPen(Qt::black));
    ui->chart->yAxis->setTickPen(QPen(Qt::black));
    ui->chart->yAxis->setSubTickPen(QPen(Qt::black));
    ui->chart->yAxis->setLabel("Deviation to base vector / %");


    // set axis ranges to show all data:
    ui->chart->xAxis->setRange(-1, defaultXWidth);
    ui->chart->yAxis->rescale(true);
}

void LineGraphWidget::setStartTimestamp(uint timestamp)
{
    startTimestamp = timestamp;
}

void LineGraphWidget::setSensorFailureFlags(const std::array<bool, MVector::size> flags)
{
    if (flags != sensorFailureFlags)
    {
        for (int i=0; i<MVector::size; i++)
        {
            if (flags[i] && !sensorFailureFlags[i])
                ui->chart->graph(i)->setVisible(false);
            else if (!flags[i] && sensorFailureFlags[i])
                ui->chart->graph(i)->setVisible(true);
        }
    }
    sensorFailureFlags = flags;

    replot();
}

void LineGraphWidget::setXAxis(double x1, double x2)
{
    ui->chart->xAxis->setRange(x1, x2);
}

void LineGraphWidget::setLogXAxis(bool logOn)
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

void LineGraphWidget::setMaxVal(double val)
{
    maxVal = val;
}

void LineGraphWidget::replot(uint timestamp)
{
    //!set new y-range
    double y_lower = 1000;
    double y_upper = 0;

    //!get current xaxis.range
    double x_axis_range_lower = ui->chart->xAxis->range().lower;
    double x_axis_range_upper = ui->chart->xAxis->range().upper;

    //iterate through graph(i) data keys and values

    for (int i = 0; i < ui->chart->graphCount(); ++i) {
        // ignore invisible graphs
        if (!ui->chart->graph(i)->visible())
            continue;

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
        if (y_lower > -0.6*yMin)
            y_lower = -yMin*0.6;

        y_upper *= 1.4;
        y_lower *= 1.2;
    }

    //set y-range
    ui->chart->yAxis->setRange(y_lower, y_upper);

    // move x-range
    if (autoMoveGraph && timestamp != 0 && timestamp >= x_axis_range_upper+startTimestamp-4 && timestamp <= x_axis_range_upper+startTimestamp)
        ui->chart->xAxis->setRange(x_axis_range_lower+2, x_axis_range_upper+2);

    // TODO:
    // redraw labels
    // find matching labels
    // draw one big label
    // --> works with current control flow?


    // delete labels if too close to each o
//    QCPRange range = ui->chart->xAxis->range();
//    int x_width = range.upper-range.lower;
//    auto iterEnd = userDefinedClassLabels.constEnd();
//     --iterEnd; // skip last item
//    for (auto iter = userDefinedClassLabels.constBegin(); iter != iterEnd; iter++)
//    {
//        int key = iter.key();
//        int nextkey = (iter+1).key();

//        if ()
//    }

    ui->chart->replot();
}

void LineGraphWidget::mousePressed(QMouseEvent * event)
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

void LineGraphWidget::dataSelected()
{
    // get selection
    QCPDataSelection selection;
    for (int i=0; i<MVector::size; i++)
        if (ui->chart->graph(i)->selected())
            selection = ui->chart->graph(0)->selection();

    // find x range of selection rectangle
    QCPRange range = ui->chart->selectionRect()->range(ui->chart->xAxis);

    ui->chart->deselectAll();

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

void LineGraphWidget::labelSelection(QMap<uint, MVector> selectionMap)
{
    // update class labels
    for (auto timestamp : selectionMap.keys())
    {
        MVector vector = selectionMap[timestamp];
        int xpos = timestamp-startTimestamp;

        // class changed?
        bool userClassChanged = false, detectedClassChanged = false;

        if (userDefinedClassLabels.contains(xpos))  // user class changed
            userClassChanged = userDefinedClassLabels[xpos]->text() != vector.userDefinedClass.getAbreviation();
        else if (!vector.userDefinedClass.isEmpty())    // vector didn't have user class before
            userClassChanged = true;

        if (detectedClassLabels.contains(xpos)) // detected class changed
            detectedClassChanged = detectedClassLabels[xpos]->text() != vector.detectedClass.getAbreviation();
        else if (!vector.detectedClass.isEmpty())   // vector didn't have detected class before
            detectedClassChanged = true;

        if (userClassChanged || detectedClassChanged)
            setLabel(xpos, vector.userDefinedClass.getAbreviation(), vector.detectedClass.getAbreviation());
    }

    ui->chart->replot();
}

bool LineGraphWidget::getUseLimits() const
{
    return useLimits;
}

void LineGraphWidget::setUseLimits(bool value)
{
    useLimits = value;
}

void LineGraphWidget::clearGraph(bool replot)
{
    // clear graphs
    for (int i=0; i<MVector::size; i++)
    {
        ui->chart->graph(i)->data()->clear();
    }
    // clear labels
    for (auto label : userDefinedClassLabels)
        ui->chart->removeItem(label);
    for (auto label : detectedClassLabels)
        ui->chart->removeItem(label);

    userDefinedClassLabels.clear();
    detectedClassLabels.clear();

    if (replot)
        ui->chart->replot();
}

void LineGraphWidget::addMeasurement(MVector measurement, uint timestamp, bool rescale)
{
    // set timestamp:
    if (ui->chart->graph(0)->data()->isEmpty())
        setStartTimestamp(timestamp);

    int xpos = timestamp-startTimestamp;
    for (int i=0; i<MVector::size; i++)
    {
        // add data point
        ui->chart->graph(i)->addData(xpos, measurement.array[i]);

        // emit sensor failures
        if (useLimits && (measurement.array[i] < minVal || measurement.array[i] > maxVal))
            emit sensorFailure(i);
    }

    qDebug() << timestamp << " : Added new Data";

    if (rescale)
        replot(timestamp);
}

void LineGraphWidget::setData(QMap<uint, MVector> map)
{
    clearGraph(false);

    if (map.isEmpty())
    {
        replot();
        return;
    }

    for (auto timestamp : map.keys())
        addMeasurement(map[timestamp], timestamp, false);

//    setXAxis(-1, defaultXWidth);
    replot();
}

void LineGraphWidget::setAutoMoveGraph(bool value)
{
    autoMoveGraph = value;
}

void LineGraphWidget::clearSelection()
{
    ui->chart->deselectAll();
}

double LineGraphWidget::getMinVal() const
{
    return minVal;
}

void LineGraphWidget::setMinVal(double value)
{
    minVal = value;
}

double LineGraphWidget::getMaxVal() const
{
    return maxVal;
}

std::array<uint, 2> LineGraphWidget::getSelection()
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

void LineGraphWidget::setLabel(int xpos, QString userDefinedBrief, QString detectedBrief)
{
    int ymax = ui->chart->yAxis->range().upper;

    // user defined class name brief
    if (userDefinedBrief != "")
    {
        QCPItemText *userLabel;

        if (userDefinedClassLabels.contains(xpos))
            userLabel = userDefinedClassLabels[xpos];
        else
        {
            userLabel = new QCPItemText(ui->chart);
            userDefinedClassLabels[xpos] = userLabel;
        }

        userLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
        userLabel->position->setType(QCPItemPosition::ptPlotCoords);
        userLabel->position->setCoords(xpos, 1.00*ymax); // place position at center/top of axis rect
        userLabel->setText(userDefinedBrief);
        userLabel->setPen(QPen(Qt::black)); // show black border around text
        userLabel->setPadding(QMargins(5,0,5,0));
    }
    // userDefinedBrief == "" && label exists: label has to be removed
    else if (userDefinedClassLabels.contains(xpos))
    {
        ui->chart->removeItem(userDefinedClassLabels[xpos]);
        userDefinedClassLabels.remove(xpos);
    }

    // detected class name brief
    if (detectedBrief != "")
    {
        QCPItemText *detectedLabel;

        if (detectedClassLabels.contains(xpos))
            detectedLabel = detectedClassLabels[xpos];
        else
        {
            detectedLabel = new QCPItemText(ui->chart);
            detectedClassLabels[xpos] = detectedLabel;
        }

        detectedLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
        detectedLabel->position->setType(QCPItemPosition::ptPlotCoords);
        detectedLabel->position->setCoords(xpos, 0.87*ymax); // place position at center/top of axis rect
        detectedLabel->setText(detectedBrief);
        detectedLabel->setPen(QPen(Qt::black)); // show black border around text
        detectedLabel->setPadding(QMargins(5,0,5,0));
    }
    // detectedBrief == "" && label exists: label has to be removed
    else if (detectedClassLabels.contains(xpos))
    {
        ui->chart->removeItem(detectedClassLabels[xpos]);
        detectedClassLabels.remove(xpos);
    }
//    // user defined class already exists
//    if (userDefinedClassLabels.contains(xpos))
//    {
//        // class should not exist
//        if (userDefinedBrief == "")
//            ui->chart->removeItem(userDefinedClassLabels[xpos])
//        // class should exist
//        else
//        {
//            //
//            userDefinedClassLabels[xpos]->setText(userDefinedBrief);
//            userDefinedClassLabels[xpos]->position->setCoords(xpos, 0.95*ymax)
//        }
//    }
//    // add the text label at the top:
//    QCPItemText *userLabel = new QCPItemText(ui->chart);
//    textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
//    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
//    textLabel->position->setCoords(0.5, 0); // place position at center/top of axis rect
//    textLabel->setText(vector.userDefinedClassBrief);
////    textLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
//    textLabel->setPen(QPen(Qt::black)); // show black border around text
}

double LineGraphWidget::getIndex(int key)
{
    auto iter = ui->chart->graph(0)->data()->findBegin(key);

    return  iter->key;
}
