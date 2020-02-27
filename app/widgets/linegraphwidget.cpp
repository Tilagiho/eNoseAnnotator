#include "linegraphwidget.h"
#include "ui_linegraphwidget.h"

#include <math.h>

#include "../classes/enosecolor.h"
#include <QTime>
LineGraphWidget::LineGraphWidget(QWidget *parent, int nChannels) :
    QWidget(parent),
    ui(new Ui::LineGraphWidget),
    nChannels(nChannels)
{
    ui->setupUi(this);

    // zero init sensorFailureFlags
    for (int i=0; i<nChannels; i++)
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
    // show label information
    connect(ui->chart, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoved(QMouseEvent*)));

    // init coordText
    coordText = new QCPItemText(ui->chart);
    coordText->setClipToAxisRect(false);
    coordText->position->setType(QCPItemPosition::PositionType::ptAbsolute);
    coordText->position->setCoords(ui->chart->width()-80, ui->chart->height()-20);
    coordText->setText("");
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

    // init graphs
    for (uint i=0; i<nChannels; i++)
    {
        ui->chart->addGraph();

        // style of plotted lines
        QColor color;
        if (nChannels == MVector::nChannels)
            color = ENoseColor::getSensorColor(i);
        else
            color = ENoseColor::getFuncColor(i);
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

    // set background
    ui->chart->setBackground(QBrush(QColor(255,250,240)));

    // set ticker colors
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
    ui->chart->yAxis->setLabel("R / R0 [%]");


    // set axis ranges to show all data:
    ui->chart->xAxis->setRange(-1, defaultXWidth);
    ui->chart->yAxis->rescale(true);
}

void LineGraphWidget::setStartTimestamp(uint timestamp)
{
    startTimestamp = timestamp;
}

void LineGraphWidget::setSensorFailureFlags(const std::array<bool, MVector::nChannels> flags)
{
    // ignore if different nChannels was set
    if (nChannels != MVector::nChannels)
        return;

    if (flags != sensorFailureFlags)
    {
        for (int i=0; i<MVector::nChannels; i++)
        {
            if (flags[i] && !sensorFailureFlags[i])
            {
                ui->chart->graph(i)->setVisible(false);
                ui->chart->graph(i)->setSelectable(QCP::SelectionType::stNone);
            }
            else if (!flags[i] && sensorFailureFlags[i])
            {
                ui->chart->graph(i)->setVisible(true);
                ui->chart->graph(i)->setSelectable(QCP::SelectionType::stDataRange);
            }
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
        emit xRangeChanged(range);
    }
}

void LineGraphWidget::resetColors()
{
    for (int i=0; i<ui->chart->graphCount(); i++)
    {
        QColor color;
        if (nChannels == MVector::nChannels)
            color = ENoseColor::getSensorColor(i);
        else
            color = ENoseColor::getFuncColor(i);

        QPen pen;
        pen.setColor(color);

        ui->chart->graph(i)->setPen(pen);
    }

    ui->chart->replot();
}

int LineGraphWidget::getNChannels() const
{
    return nChannels;
}

void LineGraphWidget::setMaxVal(double val)
{
    maxVal = val;
}

void LineGraphWidget::replot(uint timestamp)
{
    // replots are suspended
    if (!replotStatus)
        return;

    // get current xaxis.range
    auto xRange = ui->chart->xAxis->range();

//    // last and current range are bigger than full graph range
//    // -> no replot necessary
//    bool foundRange;
//    auto graphRange = ui->chart->graph(0)->data()->keyRange(foundRange);
//    if (foundRange && graphRange.lower < xRange.lower && graphRange.upper > xRange.upper && graphRange.lower < lastRange.lower && graphRange.upper > lastRange.upper)
//        return;

    // set new y-range
    double y_lower = 1000;
    double y_upper = 0;

    // iterate through all data points of first graph
    QCPGraphDataContainer::const_iterator it = ui->chart->graph(0)->data()->constBegin();
    QCPGraphDataContainer::const_iterator itEnd = ui->chart->graph(0)->data()->constEnd();

    int index = 0;
    while (it != itEnd)
    {
        // check if data point is in xRange,
        // if checkAllPoints == false, check if point was NOT in last range
        bool inXRange = it->key >= xRange.lower && it->key <= xRange.upper;
        if (inXRange)
        {
            // loop through all graphs and check for new high/ low points at index (of it->key)
            for (int i = 0; i < ui->chart->graphCount(); i++)
            {
                // ignore invisible graphs
                if (!ui->chart->graph(i)->visible())
                    continue;

                 auto data  = ui->chart->graph(i)->data()->at(index);

                 if (data->value < y_lower )
                     y_lower = data->value;
                 if (data->value > y_upper )
                     y_upper = data->value;
            }
        }

        it++;
        index++;
    }

    // log plot for big y-intervals
    if (y_upper-y_lower > 1000.0)
    {
        setLogXAxis(true);
        y_upper *= 1.1;
        if (y_lower > 0)
            y_lower *= 0.9;
        else
            y_lower *= 1.1;
    }
    // normal plot for medium y-intervals
    else if (y_lower > 100.0)
    {
        setLogXAxis(false);
        y_upper *= 1.2;
        if (y_lower > 0)
            y_lower *= 0.8;
        else
            y_lower *= 1.1;
    }
    // plot around 0 for small y-intervals
    else
    {
        if (y_lower > 0)
            y_lower *= 0.8;
        else
            y_lower *= 1.1;
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
    if (autoMoveGraph && timestamp != 0 && timestamp >= xRange.upper+startTimestamp-4 && timestamp <= xRange.upper+startTimestamp)
        ui->chart->xAxis->setRange(xRange.lower+2, xRange.upper+2);

    redrawLabels();

    ui->chart->replot();
}

void LineGraphWidget::mousePressed(QMouseEvent * event)
{
    // dragging vs selection
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
    // context menu
    else if (event->button() == Qt::RightButton)
    {
        if (coordText != nullptr)
        {
            coordText->setText("");
            ui->chart->replot(QCustomPlot::RefreshPriority::rpImmediateRefresh);
        }

        QMenu *menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);

        menu->addAction("Save Graph...", this, [this](){
            emit ImageSaveRequested();
        });

        menu->popup(ui->chart->mapToGlobal(event->pos()));
    }
}

void LineGraphWidget::mouseMoved (QMouseEvent *  event)
{
    // check if mouse is over label
    auto item = ui->chart->itemAt(event->localPos());
    if (item != 0)  // item found
    {
        // item is rectangle?
        auto focusedRect = qobject_cast<QCPItemRect*>(item);
        if (focusedRect != nullptr)
        {
            // find corresponding label string:
            QString labelString;

            // x-range of focused label
            int x1 = std::ceil(focusedRect->topLeft->coords().x());
            int x2 = std::floor(focusedRect->bottomRight->coords().x());

            // find correct label & its label string
            for (int xpos=x1; xpos<=x2; xpos++)
            {
                if (userDefinedClassLabels.contains(xpos))
                {
                    for (auto labelRect : userDefinedClassLabels[xpos].second)
                        if (focusedRect == labelRect)
                        {
                            labelString = "User Annotation:\n" + userDefinedClassLabels[xpos].first;
                            break;
                        }
                }
                if (labelString == "" && detectedClassLabels.contains(xpos))
                {
                    for (auto labelRect : detectedClassLabels[xpos].second)
                        if (focusedRect == labelRect)
                        {
                            labelString = "Detected Annotation:\n" + detectedClassLabels[xpos].first;
                            break;
                        }
                }
            }
            if (labelString != "")
            {
                labelString = labelString.split(',').join('\n');
                QToolTip::showText(event->globalPos(), labelString);
            }
        }
    }

    // check if mouse is over graph
    auto plottable = ui->chart->plottableAt(event->localPos(), true);
    if (plottable != 0)
    {
        // plottable is graph?
        auto graph = qobject_cast<QCPGraph*>(plottable);
        if (graph != nullptr)
        {
            // get channel number
            int channel = -1;
            for (int i=0; i<ui->chart->graphCount(); i++)
            {
                if (graph == ui->chart->graph(i))
                {
                    channel = i;
                    break;
                }
            }
            if (channel != -1)
            {
                if (nChannels == MVector::nChannels)
                    QToolTip::showText(event->globalPos(), "ch" + QString::number(channel+1));
                else
                    QToolTip::showText(event->globalPos(), "f" + QString::number(channel+1));
            }

        }
    }

    // get mouse position in graph coordinates
    double x = ui->chart->xAxis->pixelToCoord(event->pos().x());
    double y = ui->chart->yAxis->pixelToCoord(event->pos().y());

    // mouse outside of graph:
    // -> delete text of coordText
    auto xRange = ui->chart->xAxis->range();
    auto yRange = ui->chart->yAxis->range();
    if (y < yRange.lower || y > yRange.upper || x < xRange.lower || x > xRange.upper)
        coordText->setText("");
    // mouse inside of graph:
    // set coord text
    else
    {
        // create time string
        QString prefix = "";
        if (x < 0)
        {
            prefix = "-";
            x = -x;
        }
        int rawSecs = std::round(x);
        int hours = (rawSecs % 86400) / 3600;
        int mins = (rawSecs % 3600) / 60;
        int secs = (rawSecs % 3600) % 60;
        QTime time = QTime (hours, mins, secs);
        QString timeString = time.toString("h:mm:ss");
        // set coordinate text
        coordText->setText(prefix + timeString + ", " + QString::number(y, 'f', 1));
    }
    ui->chart->replot();
}

// adapt coordText position when resizing
void LineGraphWidget::resizeEvent(QResizeEvent* event)
{
    coordText->position->setCoords(ui->chart->width()-60, ui->chart->height()-17);
}

void LineGraphWidget::dataSelected()
{
    // get selection
    QCPDataSelection selection;
    for (int i=0; i<nChannels; i++)
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
        clearSelection();
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

        // user class changed
        if (userDefinedClassLabels.contains(xpos) && userDefinedClassLabels[xpos].first != vector.userAnnotation.toString())
            setLabel(xpos, vector.userAnnotation, true);
        // vector didn't have user class before
        else if (!userDefinedClassLabels.contains(xpos) && !vector.userAnnotation.isEmpty())
            setLabel(xpos, vector.userAnnotation, true);

        // detected class changed
        if (detectedClassLabels.contains(xpos) && detectedClassLabels[xpos].first != vector.detectedAnnotation.toString())
            setLabel(xpos, vector.detectedAnnotation, false);
        // vector didn't have user class before
        else if (!detectedClassLabels.contains(xpos) && !vector.detectedAnnotation.isEmpty())
            setLabel(xpos, vector.detectedAnnotation, false);
    }

    redrawLabels();
}

void LineGraphWidget::setIsAbsolute(bool value)
{
    isAbsolute = value;

    if (isAbsolute)
    {
        ui->chart->yAxis->setLabel("R  [kOhm]");
    }
}

bool LineGraphWidget::saveImage(const QString &filename)
{
    QStringList splitFilename = filename.split(".");

    Q_ASSERT("No file extension set!" && splitFilename.size() > 1);

    QString extension = splitFilename.last();

    bool writeOk;
    if (extension == "pdf")
        writeOk = ui->chart->savePdf(filename);
    else if (extension == "bmp")
        writeOk = ui->chart->saveBmp(filename);
    else if (extension == "jpg" || extension == "jpeg")
        writeOk = ui->chart->saveJpg(filename);
    else if (extension == "png")
        writeOk = ui->chart->savePng(filename);
    else
        Q_ASSERT("Unknown file extension!" && false);

    return writeOk;
}

QPixmap LineGraphWidget::getPixmap()
{
    return ui->chart->toPixmap();
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

        for (int i=0; i<nChannels; i++)
            ui->chart->graph(i)->setSelection(newSelection);

        int lower = qRound(ui->chart->graph(0)->data()->at(newSelection.dataRange(0).begin())->mainKey());
        int upper = qRound(ui->chart->graph(0)->data()->at(newSelection.dataRange(0).end()-1)->mainKey());

        qDebug() << "Selection synced: " << lower << ", " << upper;

        ui->chart->replot();
        emit selectionChanged(lower+startTimestamp, upper+startTimestamp);
        emit dataSelectionChanged(newSelection);
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
    for (int i=0; i<nChannels; i++)
    {
        ui->chart->graph(i)->data()->clear();
    }
    // clear labels
    for (auto label : userDefinedClassLabels)
        for (auto rectangle : label.second)
            ui->chart->removeItem(rectangle);

    for (auto label : detectedClassLabels)
        for (auto rectangle : label.second)
            ui->chart->removeItem(rectangle);

    userDefinedClassLabels.clear();
    detectedClassLabels.clear();

    if (replot)
        ui->chart->replot();
}

void LineGraphWidget::addMeasurement(MVector measurement, uint timestamp, bool rescale)
{
    Q_ASSERT(measurement.size == nChannels);

    // set timestamp:
    if (ui->chart->graph(0)->data()->isEmpty())
        setStartTimestamp(timestamp);

    // get time of last measurement
    // is used later to determine distance to current measurement
    auto endIt = ui->chart->graph(0)->data()->end();
    endIt--;
    int lastMeasKey = std::round(endIt->key);

    int xpos = timestamp-startTimestamp;
    for (int i=0; i<nChannels; i++)
    {
        // add data point
        if (!isAbsolute)
            ui->chart->graph(i)->addData(xpos, measurement[i]);
        else // isAbsolute: values / kOhm
            ui->chart->graph(i)->addData(xpos, measurement[i] / 1000);

        // emit sensor failures
        if (useLimits && (measurement[i] < minVal || measurement[i] > maxVal))
            emit sensorFailure(i);
    }

    // add annotation labels
    if (!measurement.userAnnotation.isEmpty())
        setLabel(xpos, measurement.userAnnotation, true);
    if (!measurement.detectedAnnotation.isEmpty())
        setLabel(xpos, measurement.detectedAnnotation, true);

    // if last measurement was one second ago (normally 2 seconds):
    // redraw x bounds of both labels, so they don't overlap
    if (xpos - lastMeasKey == 1)
        adjustLabelBorders(lastMeasKey, xpos);

//    qDebug() << timestamp << " : Added new Data";

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
    auto selection = ui->chart->graph(0)->selection();
    bool selected = selection.isEmpty();

    if (!ui->chart->graph(0)->selection().isEmpty())
    {
        ui->chart->deselectAll();
        ui->chart->replot();
        emit selectionCleared();
    }
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

/*!
 * \brief LineGraphWidget::setLabel creates labels from \a annotation in form of multiple QCPItemRect.
 * \param xpos
 * \param annotation
 * \a isUserAnnotation is used to determine wether a user defined or detected label should be created.
 * For class only annotations with n classes n QCPItemRect stacked on top of each other with uniform sizes are created.
 * For numeric annotations the size of each ractangle is based on their value relative to the sum of all values.
 *
 */
void LineGraphWidget::setLabel(int xpos, Annotation annotation, bool isUserAnnotation)
{
    // init labelMap dependent on isUserAnnotation
    QMap<int, QPair<QString, QList<QCPItemRect *>>>* labelMap;
    if (isUserAnnotation)
        labelMap = &userDefinedClassLabels;
    else
        labelMap = &detectedClassLabels;

    // label xpos with annotation
    if (!annotation.isEmpty())
    {
        QList<aClass> classList = annotation.getClasses();

        // remove numeric classes with value == 0.0 from classList
        // delete labels with value == 0.0
        for (int i=0; i<classList.size(); i++)
        {
            if (classList[i].getType() == aClass::Type::NUMERIC &&  qFuzzyIsNull(classList[i].getValue()))
            {
                classList.removeAt(i);
                i--;    // decrement i in order to keep i the same value for the next turn
            }
        }

        // remove old label
        if (labelMap->contains(xpos))
        {
            for (auto rect : (*labelMap)[xpos].second)
                ui->chart->removeItem(rect);
        }

        // create new label
        QList<QCPItemRect*> labels;
        for (aClass aclass : classList)
            labels << new QCPItemRect(ui->chart);
        (*labelMap)[xpos] = QPair<QString, QList<QCPItemRect*>>(annotation.toString(), labels);


        // get normation value for height of rectangles of the label
        double valueSum = 0;
        if (annotation.getType() == aClass::Type::CLASS_ONLY)
            valueSum = static_cast<double>(classList.size());
        else if (annotation.getType() == aClass::Type::NUMERIC)
            for (auto aclass : classList)
                valueSum += aclass.getValue();
        else
            Q_ASSERT ("Unknown annotation class type!" && false);

        // set properties of the rectangles forming the label
        QPair<double, double> yCoords = getLabelYCoords(isUserAnnotation);  // (lower, upper) y-Coord for the label
        double yDelta = qAbs(yCoords.second - yCoords.first); // delta between lower and uppr bound of the label
        double lastY = yCoords.second; // stores the y-coord of the bottom of the last rectangle drawn

        for (int i=0; i<classList.size(); i++)
        {
            auto rectangle = labels[i];
            auto aclass = classList[i];

            // set rectangle coords
            rectangle->topLeft->setType(QCPItemPosition::ptPlotCoords);
            rectangle->topLeft->setCoords(QPointF(xpos-1, lastY));
            rectangle->bottomRight->setType(QCPItemPosition::ptPlotCoords);

            double value = (aclass.getType() == aClass::Type::NUMERIC) ? aclass.getValue() : 1.0;   // get value of aclass
            double newY = lastY - yDelta * value / valueSum;    // newY: bottom of current rectangle, start of next rectangle
            rectangle->bottomRight->setCoords(QPointF(xpos+1, newY));
            lastY = newY;

            // set rectangle appearance
            QColor classColor = aClass::getColor(aclass);
            rectangle->setPen(QPen(classColor)); // show black border around rectangle
            rectangle->setBrush(QBrush(classColor));  // fill recatngle with the class color
            rectangle->setSelectable(true);
        }
    }
    // annotation is empty, but old label exists: delete old label
    else if (labelMap->contains(xpos))
    {
        for (auto userLabel : (*labelMap)[xpos].second)
            ui->chart->removeItem(userLabel);
        labelMap->remove(xpos);
    }
}

/*!
 * \brief LineGraphWidget::adjustLabelBorders adjusts right border of label at firstX to firstX+0.5, left border of label at secondX to secondX+0.5
 * \param firstX
 * \param secondX
 */
void LineGraphWidget::adjustLabelBorders(int firstX, int secondX)
{
    // adjust right border of first label
    if (userDefinedClassLabels.contains(firstX))
    {
        auto label = userDefinedClassLabels[firstX].second;

        for (auto rectangle : label)
            rectangle->bottomRight->setCoords(QPointF(firstX+0.5, rectangle->bottomRight->coords().y()));
    }
    if (detectedClassLabels.contains(firstX))
    {
        auto label = detectedClassLabels[firstX].second;

        for (auto rectangle : label)
            rectangle->bottomRight->setCoords(QPointF(firstX+0.5, rectangle->bottomRight->coords().y()));
    }

    // adjust left border of second label
    if (userDefinedClassLabels.contains(secondX))
    {
        auto label = userDefinedClassLabels[secondX].second;

        for (auto rectangle : label)
            rectangle->topLeft->setCoords(QPointF(secondX-0.5, rectangle->topLeft->coords().y()));
    }
    if (detectedClassLabels.contains(secondX))
    {
        auto label = detectedClassLabels[secondX].second;

        for (auto rectangle : label)
            rectangle->topLeft->setCoords(QPointF(secondX-0.5, rectangle->topLeft->coords().y()));
    }
}

void LineGraphWidget::redrawLabels()
{

    for (auto labelPair : userDefinedClassLabels)
    {
        auto label = labelPair.second;   // list of QCPItemRect

        if (!label.isEmpty())
        {
            // get relative sizes of  rectangles
            QMap<QCPItemRect*, double> rectSizeMap;
            double oldDeltaY =  qAbs(label.first()->top->pixelPosition().y() - label.last()->bottom->pixelPosition().y());

            for (auto rectangle : label)
                rectSizeMap[rectangle] = qAbs(rectangle->top->pixelPosition().y() - rectangle->bottom->pixelPosition().y()) / oldDeltaY;

            // reassign y-coords of rectangles
            QPair<double, double> yCoords = getLabelYCoords(true);  // (lower, upper) bound of label
            double deltaY = qAbs(yCoords.second - yCoords.first);
            double lastY = yCoords.second;  // stores y-coord of last rectangle bottom bound

            for (auto rectangle : label)
            {
                double newY = lastY - deltaY * rectSizeMap[rectangle];    // newY: bottom of current rectangle, start of next rectangle

                rectangle->topLeft->setCoords(QPointF(rectangle->topLeft->coords().x(), lastY));
                rectangle->bottomRight->setCoords(QPointF(rectangle->bottomRight->coords().x(), newY));

                lastY = newY;
            }
        }
    }

    for (auto labelPair : detectedClassLabels)
    {
        auto label = labelPair.second;   // list of QCPItemRect

        if (!label.isEmpty())
        {
            // get relative sizes of  rectangles
            QMap<QCPItemRect*, double> rectSizeMap;
            double oldDeltaY =  qAbs(label.first()->top->pixelPosition().y() - label.last()->bottom->pixelPosition().y());

            for (auto rectangle : label)
                rectSizeMap[rectangle] = qAbs(rectangle->top->pixelPosition().y() - rectangle->bottom->pixelPosition().y()) / oldDeltaY;

            // reassign y-coords of rectangles
            QPair<double, double> yCoords = getLabelYCoords(false);  // (lower, upper) bound of label
            double deltaY = qAbs(yCoords.second - yCoords.first);
            double lastY = yCoords.second;  // stores y-coord of last rectangle bottom bound

            for (auto rectangle : label)
            {
                double newY = lastY - deltaY * rectSizeMap[rectangle];    // newY: bottom of current rectangle, start of next rectangle

                rectangle->topLeft->setCoords(QPointF(rectangle->topLeft->coords().x(), lastY));
                rectangle->bottomRight->setCoords(QPointF(rectangle->bottomRight->coords().x(), newY));

                lastY = newY;
            }
        }
    }

    ui->chart->replot();
}

double LineGraphWidget::getIndex(int key)
{
    auto iter = ui->chart->graph(0)->data()->findBegin(key);

    return  iter->key;
}

/*!
 * \brief LineGraphWidget::getLabelYCoords returns (lower, upper) y-coordinate for user defined labels if \a isUserDefined is true, for detected labels if \a isUserDefined is false.
 * \param isUserDefined
 * \return
 */
QPair<double, double> LineGraphWidget::getLabelYCoords(bool isUserDefined)
{
    auto yRange = ui->chart->yAxis->range();

    double yBaseHeight = (yRange.upper-yRange.lower) * (1.0-labelSpace);

    if (isUserDefined)
        return QPair<double, double>(yRange.upper - labelSpace / 3.3 * yBaseHeight, yRange.upper);
    else
        return QPair<double, double>(yRange.upper - labelSpace / 3.0 * 2.0 * yBaseHeight, yRange.upper - labelSpace / 2.7 * yBaseHeight);
}

