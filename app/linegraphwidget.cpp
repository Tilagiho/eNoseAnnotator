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
    // emit xAxis range changes
    connect(ui->chart->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(onXRangeChanged(QCPRange)));

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
        QColor color = SensorColor::getSensorColor(i);
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

void LineGraphWidget::setXRange(QCPRange range)
{
    if (range != ui->chart->xAxis->range())
    {
        ui->chart->xAxis->setRange(range);
        replot();
    }
}

void LineGraphWidget::setMaxVal(double val)
{
    maxVal = val;
}

void LineGraphWidget::replot(uint timestamp)
{
    if (!replotStatus)
        return;

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
        setLogXAxis(false);

        // check for minimal y range
        if (y_upper < yMin)
            y_upper = yMin;
        if (y_lower > -0.8*yMin)
            y_lower = -yMin*0.8;

        // make space for labels
        y_upper += labelSpace * (y_upper - y_lower);
    }

    //set y-range
    ui->chart->yAxis->setRange(y_lower, y_upper);

    // move x-range
    if (autoMoveGraph && timestamp != 0 && timestamp >= x_axis_range_upper+startTimestamp-4 && timestamp <= x_axis_range_upper+startTimestamp)
        ui->chart->xAxis->setRange(x_axis_range_lower+2, x_axis_range_upper+2);

    redrawLabels();

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
    {
        selection = ui->chart->graph(i)->selection();

        if (!selection.isEmpty())
            break;
    }

    // find x range of selection rectangle
    QCPRange range = ui->chart->selectionRect()->range(ui->chart->xAxis);

    ui->chart->deselectAll();

    int lower = ceil(range.lower);
    int upper = floor(range.upper);

    qDebug() << "Selection Rect: " << lower << ", " << upper;

    if (upper<lower || selection.isEmpty())
    {
        dataSelection.clear();
        emit selectionCleared();
        return;
    }

    qDebug() << "Got selection: " << selection;


    // select all graphs in range of selection
    for (int i=0; i < ui->chart->graphCount(); i++)
    {
        ui->chart->graph(i)->setSelection(selection);
    }

    // save selection and emit signal for changed selection
    dataSelection = selection;
    emit selectionChanged(lower+startTimestamp, upper+startTimestamp);
    emit dataSelectionChanged(selection);
}

void LineGraphWidget::onXRangeChanged(QCPRange range)
{
    emit xRangeChanged(range);
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

    redrawLabels();
}

void LineGraphWidget::setIsAbsolute(bool value)
{
    isAbsolute = value;

    if (isAbsolute)
    {
        ui->chart->xAxis->setLabel("");
        ui->chart->yAxis->setLabel("Resistance / kOhm");
    }
}

void LineGraphWidget::setReplotStatus(bool value)
{
    if (!replotStatus && value)
    {
        replotStatus = true;    // has to be set before replotting

        // reset xRange to show all data
        bool foundRange;
        auto range = ui->chart->graph(0)->getKeyRange(foundRange);
        if (foundRange)
            ui->chart->xAxis->setRange(range.lower, range.upper);
        replot();
    } else
    {
        replotStatus = value;
    }
}

void LineGraphWidget::setSelection(QCPDataSelection newSelection)
{
    QCPDataSelection oldSelection = ui->chart->graph(0)->selection();

    if (newSelection.dataRange(0).end() < newSelection.dataRange(0).begin())
    {
        clearSelection();
        emit selectionCleared();
        return;
    }

    if (newSelection != oldSelection)
    {
        ui->chart->deselectAll();

        for (int i=0; i<MVector::size; i++)
            ui->chart->graph(i)->setSelection(newSelection);

        int lower = qRound(ui->chart->graph(0)->data()->at(newSelection.dataRange(0).begin())->mainKey());
        int upper = qRound(ui->chart->graph(0)->data()->at(newSelection.dataRange(0).end()-1)->mainKey());

        qDebug() << "Selection synced: " << lower << ", " << upper;

        ui->chart->replot();
        emit selectionChanged(lower+startTimestamp, upper+startTimestamp);
    }
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
    for (auto label : joinedUserDefinedClassLabels)
        ui->chart->removeItem(label);
    for (auto label : joinedDetectedClassLabels)
        ui->chart->removeItem(label);

    userDefinedClassLabels.clear();
    detectedClassLabels.clear();
    joinedUserDefinedClassLabels.clear();
    joinedDetectedClassLabels.clear();

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
        if (!isAbsolute)
            ui->chart->graph(i)->addData(xpos, measurement.array[i]);
        else // isAbsolute: values / kOhm
            ui->chart->graph(i)->addData(xpos, measurement.array[i] / 1000);


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
    ui->chart->replot();
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
    auto xRange = ui->chart->xAxis->range();
    auto yRange = ui->chart->yAxis->range();

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
        userLabel->position->setCoords(xpos, getLabelYCoord(true)); // place position at center/top of axis rect
        userLabel->setText(userDefinedBrief);
        userLabel->setPen(QPen(Qt::black)); // show black border around text

        // set width
        int xAxisWidth = ui->chart->xAxis->axisRect()->width(); // width of xAxis in pixel
        double relativeClassWidth = 1 / (xRange.upper - xRange.lower);    // relative width of current successive matching class to current xRange
        int margin = qRound(relativeClassWidth * xAxisWidth / 2.0);
        userLabel->setPadding(QMargins(margin,0,margin,0));
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
        detectedLabel->position->setCoords(xpos, getLabelYCoord(false)); // place position at center/top of axis rect
        detectedLabel->setText(detectedBrief);
        detectedLabel->setPen(QPen(Qt::black)); // show black border around text

        // set width
        int xAxisWidth = ui->chart->xAxis->axisRect()->width(); // width of xAxis in pixel
        double relativeClassWidth = 1 / (xRange.upper - xRange.lower);    // relative width of current successive matching class to current xRange
        int margin = qRound(relativeClassWidth * xAxisWidth / 2.0);
        detectedLabel->setPadding(QMargins(margin,0,margin,0));
    }
    // detectedBrief == "" && label exists: label has to be removed
    else if (detectedClassLabels.contains(xpos))
    {
        ui->chart->removeItem(detectedClassLabels[xpos]);
        detectedClassLabels.remove(xpos);
    }
}

void LineGraphWidget::redrawLabels()
{
    // no work to do if no labels exist
    if (userDefinedClassLabels.isEmpty() && detectedClassLabels.isEmpty())
        return;

   for (auto label : joinedUserDefinedClassLabels)
       ui->chart->removeItem(label);
    joinedUserDefinedClassLabels.clear();

    for (auto label : joinedDetectedClassLabels)
        ui->chart->removeItem(label);
     joinedDetectedClassLabels.clear();

    QCPRange xRange = ui->chart->xAxis->range();

    // ___ user defined labels ___
    QCPItemText* userBeginLabel = nullptr;
    QCPItemText* detectedBeginLabel = nullptr;    // marks the begin of labels of labels with the current class
    QList<QList<QCPItemText*>> matchingUserLabels, matchingDetectedLabels;
    QList<QCPItemText*> currentUserMatches, currentDetectedLabels;

    auto keyRange = ui->chart->graph(0)->data()->dataRange();

    // go through all data
    for (int sortKey=keyRange.begin(); sortKey<keyRange.end(); sortKey++)
    {
        int xpos = qRound(ui->chart->graph(0)->data()->at(sortKey)->mainKey());

        // xpos in xRange+-1
        if (xRange.lower-1 <= xpos && xpos <= xRange.upper+1)
        {
            // --- user defined labels ---
            // user label at xpos exists
            if (userDefinedClassLabels.contains(xpos))
            {
                auto currentLabel = userDefinedClassLabels[xpos];
                currentLabel->setVisible(true);

                // set first beginlabel
                if (userBeginLabel == nullptr)
                    userBeginLabel = currentLabel;

                // matching classes
                if (userBeginLabel->text() == currentLabel->text())
                    currentUserMatches << currentLabel;
                // new class begins
                // store current matches & begin next match
                else
                {
                    matchingUserLabels << currentUserMatches;
                    currentUserMatches.clear();
                    userBeginLabel = currentLabel;
                    currentUserMatches << currentLabel;
                }
            }
            // no label at xpos
            else
            {
                // save last matches
                if (currentUserMatches.size() > 1)
                {
                    matchingUserLabels << currentUserMatches;
                }
                // reset matching labels
                currentUserMatches.clear();
                userBeginLabel = nullptr;
            }

            // --- detected labels ---
            // detected label at xpos exists
            if (detectedClassLabels.contains(xpos))
            {
                auto currentLabel = detectedClassLabels[xpos];
                currentLabel->setVisible(true);

                // set first beginlabel
                if (detectedBeginLabel == nullptr)
                    detectedBeginLabel = currentLabel;

                // matching classes
                if (currentLabel->text() == detectedBeginLabel->text())
                    currentDetectedLabels << currentLabel;
                // new class begins
                // store current matches & begin next match
                else
                {
                    matchingDetectedLabels << currentDetectedLabels;
                    currentDetectedLabels.clear();
                    detectedBeginLabel = currentLabel;
                    currentDetectedLabels << currentLabel;
                }
            }
            // no label at xpos or not in xRange
            else
            {
                // save last matches
                if (currentDetectedLabels.size() > 1)
                {
                    matchingDetectedLabels << currentDetectedLabels;
                }
                // reset matching labels
                currentDetectedLabels.clear();
                detectedBeginLabel = nullptr;
            }
        }
    }

    // store last match
    matchingUserLabels << currentUserMatches;
    matchingDetectedLabels << currentDetectedLabels;

    // go through user matches & create joined labels
    for (auto matchList : matchingUserLabels)
    {
        if (matchList.size() > 1)
        {
            // hide single labels
            for (auto label : matchList)
                label->setVisible(false);
            QCPItemText* joinedUserLabel = new QCPItemText(ui->chart);

            joinedUserLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
            joinedUserLabel->position->setType(QCPItemPosition::ptPlotCoords);
            joinedUserLabel->setText(matchList.first()->text());
            joinedUserLabel->setPen(QPen(Qt::black)); // show black border around text

            // set coords
            int beginX = matchList.first()->positions()[0]->coords().x();
            int endX = matchList.last()->positions()[0]->coords().x();

            double xmid = (beginX+endX)/2.0;
            joinedUserLabel->position->setCoords(xmid, getLabelYCoord(true)); // place position at center/top of axis rect

            // set width
            int xAxisWidth = ui->chart->xAxis->axisRect()->width(); // width of xAxis in pixel
            double relativeClassWidth = (endX - beginX + 1) / (xRange.upper - xRange.lower);    // relative width of current successive matching class to current xRange
            int margin = qRound(relativeClassWidth * xAxisWidth / 2.0);
            joinedUserLabel->setPadding(QMargins(margin,0,margin,0));

            joinedUserDefinedClassLabels[xmid] = joinedUserLabel;
        }
    }

    // go through defined matches & create joined labels
    for (auto matchList : matchingDetectedLabels)
    {
        if (matchList.size() > 1)
        {
            // hide single labels
            for (auto label : matchList)
                label->setVisible(false);
            QCPItemText* joinedDetectedLabel = new QCPItemText(ui->chart);

            joinedDetectedLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
            joinedDetectedLabel->position->setType(QCPItemPosition::ptPlotCoords);
            joinedDetectedLabel->setText(matchList.first()->text());
            joinedDetectedLabel->setPen(QPen(Qt::black)); // show black border around text

            // set coords
            int beginX = matchList.first()->positions()[0]->coords().x();
            int endX = matchList.last()->positions()[0]->coords().x();

            double xmid = (beginX+endX)/2.0;
            joinedDetectedLabel->position->setCoords(xmid, getLabelYCoord(false)); // place position at center/top of axis rect

            // set width
            int xAxisWidth = ui->chart->xAxis->axisRect()->width(); // width of xAxis in pixel
            double relativeClassWidth = (endX - beginX + 1) / (xRange.upper - xRange.lower);    // relative width of current successive matching class to current xRange
            int margin = qRound(relativeClassWidth * xAxisWidth / 2.0);

            joinedDetectedLabel->setPadding(QMargins(margin,0,margin,0));

            joinedDetectedClassLabels[xmid] = joinedDetectedLabel;
        }
    }

    ui->chart->replot();
}

double LineGraphWidget::getIndex(int key)
{
    auto iter = ui->chart->graph(0)->data()->findBegin(key);

    return  iter->key;
}

double LineGraphWidget::getLabelYCoord(bool isUserDefined)
{
    auto yRange = ui->chart->yAxis->range();

    double yBaseHeight = (yRange.upper-yRange.lower) / (1.0+labelSpace);

    if (isUserDefined)
        return yRange.upper;
    else
        return yRange.upper - labelSpace / 2.0 * yBaseHeight;
}
