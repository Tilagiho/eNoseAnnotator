#include "linegraphwidget.h"

#include "../classes/defaultSettings.h"
#include "../classes/measurementdata.h"
#include "../classes/enosecolor.h"
#include "../classes/defaultSettings.h"

#include <float.h>

#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_painter.h>
#include <qwt_plot_panner.h>

#include <qwt_picker_machine.h>

#include <qwt_date_scale_draw.h>
#include <qwt_date_scale_engine.h>

#include <qwt_plot_marker.h>

#include <QMouseEvent>

/*!
 * \brief The CurveData class is a container for the data of one curve. It enables appending data to the curve.
 * Based on qwt example "realtime".
 */
CurveData::CurveData()
{
    clear();    // init d_bounding_rect
}

QRectF CurveData::boundingRect() const
{
    if ( d_boundingRect.width() < 0.0 )
        d_boundingRect = qwtBoundingRect( *this );

    return d_boundingRect.normalized();
}

inline void CurveData::append( const QPointF &point )
{
    d_samples += point;

    // resize bounding rectangle, if necessary, to contain point:
    // init bounding rectangle
    if ( qFuzzyCompare(d_boundingRect.width(), 0.0) && qFuzzyCompare(d_boundingRect.height(), 0.0) )
        d_boundingRect = qwtBoundingRect( *this );

    if (!d_boundingRect.contains(point))
    {
        d_boundingRect = d_boundingRect | QRectF(d_boundingRect.center(), point);
        d_boundingRect = d_boundingRect.normalized();
    }
}

void CurveData::clear()
{
    d_samples.clear();
    d_samples.squeeze();
    d_boundingRect = QRectF( 0.0, 0.0, 0.0, 0.0 );
}

QVector<QPointF>* CurveData::samples()
{
    return &d_samples;
}

QwtText ToolTipPlotPicker::trackerTextF ( const QPointF & pos ) const
{
    emit mouseMoved(pos);

    // get tooltip text
    QwtText text;

    auto plt = plot();

    // check annotation rects
    if (text.isEmpty())
    {
        QList<QwtPlotItem*> rectList = plt->itemList(1001); // AClassRectItem

        for (auto item : rectList)
        {
            auto rect = static_cast<AClassRectItem*>(item);
            if ( rect->boundingRect().contains(pos) )
            {
                QString annotationText;
                annotationText += rect->getIsUserAnnotation() ? "User annotation:\n" : "Detected annotation:\n";
                annotationText += rect->getAnnotation().toString().split(",").join("\n");
                text.setText(annotationText);
                text.setRenderFlags(Qt::AlignLeft);
                break;
            }
        }
    }

    // check curves
    if (text.isEmpty())
    {
        QList<QwtPlotItem*> curveList = plt->itemList(QwtPlotItem::Rtti_PlotCurve);

        double minDistance = qInf();
        int closestItemIndex = -1;
        int closestPointIndex;
        int point_index;

        for (int i=0; i<curveList.size(); i++)
        {
            auto item = curveList[i];
            double distance = qInf();

            if (item->isVisible())
            {
                auto curve = dynamic_cast<QwtPlotCurve*>(item);
                auto t_pos = transform(pos);
                point_index = curve->closestPoint(t_pos, &distance);

                QPointF curvePoint;
                if (curve->data()->size() > 0)
                    curvePoint = curve->sample(0);

                if (point_index < 0)
                    continue;

                if (distance < minDistance)
                {
                    minDistance = distance;
                    closestItemIndex = i;
                    closestPointIndex = point_index;
                }
            }
        }

        // check if pos is close to curve
        auto dummyPoint = pos;
        dummyPoint.setX(dummyPoint.x() + 6000);
        double threshold = QLineF(transform(pos), transform(dummyPoint)).length();

        if(minDistance > threshold) { return QwtText(); }

        // if close: return curve name
        auto closestItem = curveList[closestItemIndex];
        text = closestItem->title();
    }

    QColor bg( Qt::white );
    bg.setAlpha( 200 );
    text.setBackgroundBrush( QBrush( bg ) );

    return text;
}

class DateScaleDraw: public QwtDateScaleDraw
{
public:
    DateScaleDraw( Qt::TimeSpec timeSpec ):
        QwtDateScaleDraw( timeSpec )
    {
        setDateFormat( QwtDate::Millisecond, "hh:mm:ss" );
        setDateFormat( QwtDate::Second, "hh:mm:ss" );
        setDateFormat( QwtDate::Minute, "hh:mm" );
        setDateFormat( QwtDate::Hour, "hh:mm\nddd dd MMM" );
        setDateFormat( QwtDate::Day, "ddd dd MMM" );
        setDateFormat( QwtDate::Week, "dd MMM yy" );
        setDateFormat( QwtDate::Month, "MMM yy" );
    }
};

/*!
 * \brief AClassRectItem::AClassRectItem draws a rectangular item for displaying aclass.
 * The position of the rectangle is determined by the interval set (x) and by the position of aClass in annotation (y).
 * When drawing all aClasses of an annotation with an AClassRectItem the top margin of the graph is split into 7 parts.
 * Detected annotations are drawn into the 2nd and 3rd part, user annotations into the 5th and 6th.
 *
 * \param interval
 * \param annotation
 * \param aclass
 * \param isUserAnnotation
 */
AClassRectItem::AClassRectItem(QwtInterval xInterval, Annotation annotation, aClass aclass, bool isUserAnnotation):
    xInterval(xInterval),
    annotation(annotation),
    aclass(aclass),
    isUserAnnotation(isUserAnnotation)
{
    Q_ASSERT(annotation.contains(aclass));
    setZ( 10 );
}

void AClassRectItem::setInterval( QwtInterval& xValue)
{
    xInterval = xValue;
}

double AClassRectItem::left()
{
    return xInterval.minValue();
}

void AClassRectItem::setLeft(const double &value)
{
    xInterval.setMinValue(value);
}

double AClassRectItem::right()
{
    return xInterval.maxValue();
}

void AClassRectItem::setRight(const double &value)
{
    xInterval.setMaxValue(value);
}

void AClassRectItem::draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect) const
{
    double x1 = xInterval.minValue();
    int px1 = qRound( xMap.transform( x1 ) );
    double x2 = xInterval.maxValue();
    int px2 = qRound( xMap.transform( x2 ) );

    // determine y range of class rect
    auto plt = static_cast<LineGraphWidget*>(plot());

    auto plot_b_rect = plt->boundingRect();
    auto yInterval = QwtInterval( plot_b_rect.top(), plot_b_rect.bottom() );

    double windowTop = yInterval.maxValue();
    double windowHeight = yInterval.maxValue() - yInterval.minValue();

    double yOffset = (isUserAnnotation) ? 0 : 4;
    double annotationTop = windowTop - (7 + yOffset) / 14. * windowHeight * LGW_Y_RELATIVE_MARGIN;
    double annotationBottom = windowTop - (10 + yOffset) / 14. * windowHeight * LGW_Y_RELATIVE_MARGIN;

    double y1 = -1., y2 = -1;
    int py1 = -1, py2 = -1;
    auto classList = annotation.getClasses();
    // sum up values
    double valueSum = 0.;
    if (annotation.getType() == aClass::Type::CLASS_ONLY)
        valueSum = classList.size();
    else
        for ( aClass annoClass : classList)
            valueSum += annoClass.getValue();

    double cumulatedSum = 0.;
    for ( aClass annoClass : classList)
    {
        double value = (annoClass.getType() == aClass::Type::NUMERIC) ? annoClass.getValue() : 1.;

        if (annoClass == aclass)
        {
            y1 = annotationTop + cumulatedSum / valueSum * (annotationBottom - annotationTop);
            py1 = qRound( yMap.transform( y1 ) );
            y2 = annotationTop + (cumulatedSum + value) / valueSum * (annotationBottom - annotationTop);
            py2 = qRound( yMap.transform( y2 ) );
            break;
        }
        cumulatedSum += value;
    }
    b_rect = QRectF(x1, y1, x2-x1, y2-y1);
    painter->fillRect( QRect(px1, py1, px2 - px1, py2 - py1), QBrush(getColor()));
}

QRectF AClassRectItem::boundingRect()
{
    return b_rect;
}

int AClassRectItem::rtti() const
{
    return 1001;
}

Annotation AClassRectItem::getAnnotation() const
{
    return annotation;
}

bool AClassRectItem::getIsUserAnnotation() const
{
    return isUserAnnotation;
}

QColor AClassRectItem::getColor() const
{
    return aClass::getColor(aclass);
}

LineGraphWidget::LineGraphWidget(QWidget *parent) :
    QwtPlot(parent),
    rectangleZoom(new FixedPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, canvas())),
    zonePicker(new QwtPlotPicker(canvas())),
    zoneItem(new QwtPlotZoneItem()),
    toolTipPicker(new ToolTipPlotPicker(canvas())),
    coordinateLabel(new QwtPlotTextLabel),
    legend(new QwtPlotLegendItem)
{
    setCanvasBackground(QBrush(GRAPH_BACKGROUND_COLOR));

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setMajorPen(QPen(Qt::DotLine));
    grid->attach( this );

    // setup x axis
    QwtDateScaleDraw *dateScaleDraw = new DateScaleDraw( Qt::TimeSpec::LocalTime );
    QwtDateScaleEngine *dateScaleEngine = new QwtDateScaleEngine( Qt::TimeSpec::LocalTime );
    dateScaleEngine->setAttribute(QwtScaleEngine::Floating, true);
    dateScaleEngine->setMargins(zoomBaseOffset.x(), zoomBaseOffset.x());

    setAxisScaleDraw( QwtPlot::xBottom, dateScaleDraw );
    setAxisScaleEngine( QwtPlot::xBottom, dateScaleEngine );
    setAxisLabelAlignment( QwtPlot::xBottom, Qt::AlignCenter | Qt::AlignBottom );

    setAxisTitle(QwtPlot::xBottom, "Time/ Date of measurement");

    // set range of x-axis to current time
    auto currentDatetime = QDateTime::currentDateTime();
    int prevSecs = LGW_AUTO_MOVE_ZONE_SIZE;
    setPrevXRange(currentDatetime.addSecs(prevSecs), prevSecs);

    // setup y axis
    QwtLinearScaleEngine *yScaleEngine = new QwtLinearScaleEngine();
    yScaleEngine->setMargins(zoomBaseOffset.y(), zoomBaseOffset.y());
    yScaleEngine->setAttribute(QwtScaleEngine::Floating, true);
    yScaleEngine->setAttribute(QwtScaleEngine::Inverted, false);

    setAxisScaleEngine( QwtPlot::yLeft, yScaleEngine );

    // panner: drag and drop the graph range
    auto panner = new QwtPlotPanner(canvas());
    panner->setMouseButton(Qt::LeftButton, Qt::NoModifier);

    // mouseWheelZoom: zoom with mousewheel
    QwtPlotMagnifier *mouseWheelZoom = new FixedPlotMagnifier(canvas());
    mouseWheelZoom->setMouseButton(Qt::MouseButton::NoButton);
//    mouseWheelZoom->setAxisEnabled(QwtPlot::yLeft, false);

    // rectangle zoom: zoom with SHIFT + left mouse rectangle
    rectangleZoom->setRubberBand( QwtPicker::RectRubberBand );
    rectangleZoom->setTrackerMode(QwtPicker::AlwaysOff);

    rectangleZoom->setMousePattern(QwtEventPattern::MouseSelect1,Qt::LeftButton, Qt::ShiftModifier);
    rectangleZoom->setMousePattern( QwtEventPattern::MouseSelect2,Qt::RightButton, Qt::ShiftModifier); //zoom out by 1
    rectangleZoom->setMousePattern( QwtEventPattern::MouseSelect3,Qt::MiddleButton, Qt::ShiftModifier); //zoom out by 1

    // zone picker: pick selection with CTRL + left mouse rectangle
    zonePicker->setRubberBand(QwtPicker::RubberBand::RectRubberBand);
    zonePicker->setStateMachine(new QwtPickerDragRectMachine);
    zonePicker->setTrackerMode(QwtPicker::AlwaysOff);
    zonePicker->setMousePattern(QwtEventPattern::MouseSelect1,Qt::LeftButton, Qt::ControlModifier);
    connect(zonePicker, SIGNAL(selected(const QRectF &)), this, SLOT(makeSelection(const QRectF &)));

    // zoneItem: used to show selection
    zoneItem->setBrush(QBrush(QColor(Qt::blue)));
    zoneItem->setOrientation(Qt::Orientation::Vertical);
    zoneItem->setVisible(false);
    zoneItem->attach(this);

    // tooltip: show curve name
    toolTipPicker->setTrackerMode(QwtPicker::AlwaysOn);
    setMouseTracking(true);
    connect(toolTipPicker, &ToolTipPlotPicker::mouseMoved, this, &LineGraphWidget::setMouseCoordinates);

    // mouse coordinate label
    coordinateLabel->setText( QwtText() );
    coordinateLabel->attach( this );

    // axis synchronisation
    QObject::connect((QwtScaleWidget*) axisWidget(QwtPlot::xBottom) , &QwtScaleWidget::scaleDivChanged, this, [this](){
        emit axisIntvSet(axisInterval(QwtPlot::xBottom), QwtPlot::xBottom);
    });

    // functionalisation legend
    legend->setAlignment(Qt::AlignRight | Qt::AlignTop);
    legend->attach(this);
    legend->setBorderPen(QPen(Qt::black));
    QColor lbg =canvasBackground().color();
    lbg.setAlpha(200);
    legend->setBackgroundBrush(lbg);
}

LineGraphWidget::~LineGraphWidget()
{
    clearGraph();
}

void LineGraphWidget::clearGraph()
{
    for (int i=0; i<dataCurves.size(); i++)
    {
        delete dataCurves[i];
        delete selectionCurves[i];
    }

    for (auto label : userDefinedClassLabels)
        for (auto rect : label)
            delete rect;

    for (auto label : detectedClassLabels)
        for (auto rect : label)
            delete rect;

    dataCurves.clear();
    selectionCurves.clear();
    userDefinedClassLabels.clear();
    detectedClassLabels.clear();
    legend->clearLegend();

    if (replotStatus)
        replot();
}

void LineGraphWidget::clearSelection()
{
    if (zoneItem->isVisible())  // currently data selected
    {
        zoneItem->setInterval(0., 0.);
        zoneItem->setVisible(false);

        for (auto selectionCurve : selectionCurves)
        {
            CurveData *selectionCurveData = static_cast<CurveData *>( selectionCurve->data() );
            selectionCurveData->clear();
        }

        if ( replotStatus )
            replot();

        emit selectionCleared();
    }
}

void LineGraphWidget::setSensorFailures(const std::vector<bool> &sensorFailures, const Functionalisation &functionalisation)
{
    if (dataCurves.size() == 0)
        return;

    Q_ASSERT( sensorFailures.size() == dataCurves.size() );

    for (int i=0; i<sensorFailures.size(); i++)
    {
        dataCurves[i]->setVisible(!sensorFailures[i]);
        selectionCurves[i]->setVisible(!sensorFailures[i]);
    }

    if (replotStatus)
    {
        setupLegend(functionalisation, sensorFailures);
        replot();
        setZoomBase();
    }
}

void LineGraphWidget::setFunctionalisation(const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    for (int i=0; i<dataCurves.size(); i++)
    {
        auto curve = dataCurves[i];
        QColor graphColor = getGraphColor(i, functionalisation);

        curve->setPen(graphColor);
        curve->setStyle( QwtPlotCurve::CurveStyle::Lines );
        curve->setSymbol( new QwtSymbol( QwtSymbol::Ellipse,
                                           QBrush(graphColor), QPen(graphColor), QSize( 4, 4 ) ) );
    }

    for (int i=0; i<selectionCurves.size(); i++)
    {
        auto curve = selectionCurves[i];
        QColor graphColor = getGraphColor(i, functionalisation);

        curve->setPen(graphColor);
        curve->setStyle( QwtPlotCurve::CurveStyle::Lines );
        curve->setSymbol( new QwtSymbol( QwtSymbol::Ellipse,
                                           QBrush(graphColor), QPen(graphColor), QSize(6, 6) ) );
    }

    setupLegend(functionalisation, sensorFailures);
    replot();
}

void LineGraphWidget::setAnnotations(const QMap<uint, Annotation> &annotations, bool isUserAnnotation)
{
    for (uint timestamp : annotations.keys())
        setLabel(timestamp, annotations[timestamp], isUserAnnotation);

    adjustLabels(isUserAnnotation);
    if (replotStatus)
        replot();
}

void LineGraphWidget::setMouseCoordinates(const QPointF &coords)
{
    auto t_coords = QPointF(transform(QwtPlot::xBottom, coords.x()), transform(QwtPlot::yLeft, coords.y()));

    double w = canvas()->width();
    double h =canvas()->height();
    const double thresh = 0.04;

    // hide coordinate label at boundaries of the canvas
    if (t_coords.x() < thresh * w || t_coords.x() > (1 - thresh) * w || t_coords.y() < thresh * h || t_coords.y() > (1 - thresh) * h)
        coordinateLabel->setText(QwtText());
    else
    {
        auto datetime = QwtDate::toDateTime(coords.x(), Qt::TimeSpec::LocalTime);
        QwtText title( "x: " + datetime.toString(locale().dateTimeFormat()) + "\ny: " + QString::number(coords.y()));

        QColor bg = canvasBackground().color();
        bg.setAlpha( 200 );
        title.setBackgroundBrush( QBrush( bg ) );

        title.setRenderFlags( Qt::AlignLeft | Qt::AlignTop );
        coordinateLabel->setText(title);
    }

    replot();
}

void LineGraphWidget::zoomToData()
{
    QRectF b_Rect = boundingRect();

    setAxisIntv(QwtInterval(b_Rect.left(), b_Rect.right()), QwtPlot::xBottom);
    setAxisIntv(QwtInterval(b_Rect.top(), b_Rect.bottom()), QwtPlot::yLeft);
}

void LineGraphWidget::autoScale (bool xAxis, bool yAxis)
{
    auto bounding_rect = boundingRect();
    if (xAxis)
        setAxisIntv(QwtInterval(bounding_rect.left(), bounding_rect.right()), QwtPlot::xBottom);
    if (yAxis)
        setAxisIntv(QwtInterval(bounding_rect.top(), bounding_rect.bottom()), QwtPlot::yLeft);
}

void LineGraphWidget::setAxisIntv (QwtInterval intv, QwtPlot::Axis axis)
{
    auto currentIntv = axisInterval(axis);

    if (intv != currentIntv)
    {
        setAxisScale(axis, intv.minValue(), intv.maxValue());
        if (replotStatus)
            replot();
    }
}

void LineGraphWidget::setPrevXRange(QDateTime datetime, int prevSeconds)
{
    QwtInterval intv(getT(datetime.addSecs(-prevSeconds)), getT(datetime));
    setAxisIntv(intv, QwtPlot::xBottom);
    autoScale(false, true);
//    setAutoScale(false, true);

    if (replotStatus)
        replot();
}

void LineGraphWidget::shiftXRange(double seconds)
{
    QwtInterval intv = axisInterval(QwtPlot::xBottom);
    intv.setMinValue(intv.minValue() + seconds * 1000.);
    intv.setMaxValue(intv.maxValue() + seconds * 1000.);

    setAxisIntv(intv, QwtPlot::xBottom);
    if (replotStatus)
        replot();
}

/*!
 * \brief LineGraphWidget::makeSelection makes selection in graph based on the x-range of rect.
 * The x-coordinates of the rect have to be in the ms format. This format is produced by getT().
 * \param rect
 */
void LineGraphWidget::makeSelection(const QRectF &rect)
{
    // get rectangle x interval
    double minT = rect.left();
    double maxT = rect.right();

    if (maxT < minT)
        std::swap(minT, maxT);

    makeSelection(minT, maxT);
}

/*!
 * \brief LineGraphWidget::makeSelection makes selection in graph in the range [minT; maxT].

 * \param minT lower bound in the ms format produced by getT()
 * \param maxT upper bound in the ms format produced by getT()
 */
void LineGraphWidget::makeSelection(double minT, double maxT)
{
    makeSelection(getTimestamp(minT), getTimestamp(maxT));
}

/*!
 * \brief LineGraphWidget::makeSelection makes selection in graph in the range [lower; upper].
 * \param lower lower bound in the timestamp format produced by getTimestamp()
 * \param upper upper bound in the timestamp format produced by getTimestamp()
 */
void LineGraphWidget::makeSelection(uint lower, uint upper)
{
    auto zoneIntv = zoneItem->interval();

    double minT = getT( lower );
    double maxT = getT( upper );

    if (!qFuzzyCompare(minT, zoneIntv.minValue()) || !qFuzzyCompare(maxT, zoneIntv.maxValue()))
    {
        if (selectPoints(minT, maxT))
        {
            zoneItem->setInterval(minT, maxT);
            zoneItem->setVisible(true);

            replot();
            emit selectionMade(lower, upper);
        }
        else
            emit selectionCleared();
    }
}


bool LineGraphWidget::selectPoints(double min, double max)
{
    clearSelection();

    if (dataCurves.isEmpty())
        return false;

    // find selected points
    auto data = static_cast<CurveData*>(dataCurves[0]->data())->samples()->toStdVector();

    auto startElement = std::find_if(data.begin(), data.end(), [min](QPointF p){
        return p.rx() > min;
    });
    if (startElement == data.end()) // selection starts right of curve
        return false;

    auto endElement = std::find_if(startElement, data.end(), [max](QPointF p){
        return p.rx() > max;
    });
    if (endElement == data.begin()) // selection ends left of curve
        return false;

    int startIndex = std::distance(data.begin(), startElement);
    int endIndex = std::distance(data.begin(), endElement);

    if (startIndex == endIndex)
        return false;

    setReplotStatus(false);
    for (int i=0; i<dataCurves.size(); i++)
    {
        QwtPlotCurve* curve  = selectionCurves[i];

        for (int pos=startIndex; pos<endIndex; pos++)
        {
            QPointF point = dataCurves[i]->data()->sample(pos);

            addPoint(curve, point);
        }
    }
    setReplotStatus(true);

    return true;
}

bool LineGraphWidget::autoMoveXRange(double t)
{
    if (measRunning)
    {
        auto intv = axisInterval(QwtPlot::xBottom);
        double delta = intv.maxValue() - t;
        double ratio_threshold = LGW_AUTO_MOVE_ZONE_BORDER_RATIO * intv.width();
        double min_threshold = LGW_AUTO_MOVE_ZONE_BORDER_MIN * 1000;
        if (delta > 0)
        {
            if (delta < ratio_threshold)
            {
//                setReplotStatus(false); // set replotStatus to false in order to replot after points were added
                shiftXRange((ratio_threshold - delta) / 1000.);
                return true;
            }
            else if (delta < min_threshold)
            {
//                setReplotStatus(false); // set replotStatus to false in order to replot after points were added
                shiftXRange((min_threshold - delta) / 1000.);
                return true;
            }
        }
    }

    return false;
}

QRectF LineGraphWidget::boundingRect() const
{
    QRectF rect(0., 0., -1., -1.);
    for (auto curve : dataCurves)
    {
        // ignore hidden channels
        if (!curve->isVisible())
            continue;

        auto data = static_cast<CurveData*>(curve->data());
        if (rect.width() < 0.)
            rect = data->boundingRect();
        else
            rect = rect | data->boundingRect();
    }

    return rect;
}

double LineGraphWidget::getT(double timestamp)
{
    uint roundedTimestamp = qRound(timestamp);
    return getT(roundedTimestamp) + 1000. * (timestamp - roundedTimestamp);
}

double LineGraphWidget::getT(uint timestamp)
{
    return getT( QDateTime::fromTime_t(timestamp) );
}

double LineGraphWidget::getT(QDateTime datetime)
{
    return QwtDate::toDouble(datetime);
}

uint LineGraphWidget::getTimestamp(double t)
{
    return QwtDate::toDateTime(t).toTime_t();
}

/*!
 * \brief LineGraphWidget::setAxisScale calls QwtPlot::setAxisScale, but converts inverted ranges (min > max) into non-inverted ranges.
 * Necessary, because setZoomBase sometimes inverses the y-scale for some reason.
 * Call QwtPlot::setAxisScale directly if inverted scales should be allowed.
 * \param axisId
 * \param min
 * \param max
 * \param stepSize
 */
void LineGraphWidget::setAxisScale(int axisId, double min, double max, double stepSize)
{
    if (min > max)
        std::swap(min, max);

    QwtPlot::setAxisScale(axisId, min, max, stepSize);
}

void LineGraphWidget::setupLegend(const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    auto funcMap = functionalisation.getFuncMap(sensorFailures);
    QMap<int, QwtPlotCurve*> legendCurves;

    for (int i=0; i<dataCurves.size(); i++)
    {
        // hide all curves from legend
        dataCurves[i]->setItemAttribute(QwtPlotItem::Legend, false);
        updateLegend(dataCurves[i]);

        selectionCurves[i]->setItemAttribute(QwtPlotItem::Legend, false);
        updateLegend(selectionCurves[i]);
        int func = functionalisation[i];

        // add one data curve for each functionalisation to be added to the legend
        // channels with sensorFailures are ignored
        if (!sensorFailures[i] && funcMap.contains(func) && funcMap[func] > 0)
        {
            // add curve to legend curves
            legendCurves[func] = dataCurves[i];

            // set count in funcMap to zero
            // -> future curves with the same functionalisation are ignored
            funcMap[func] = 0;
        }
    }

    // add legendCurves to legend
    for (int func : legendCurves.keys())
    {
        auto curve = legendCurves[func];

        // reattach legendCurves so they appear in the desired order
        curve->detach();
        curve->attach(this);

        // update legend data:
        // display functionalisation of channel instead of title
        QwtLegendData data;

        QwtText label = "f" + QString::number(func);
        label.setRenderFlags( label.renderFlags() & Qt::AlignLeft );

        QVariant titleValue;
        qVariantSetValue( titleValue, label );
        data.setValue( QwtLegendData::TitleRole, titleValue );

        const QwtGraphic graphic = curve->legendIcon( 0, curve->legendIconSize() );
        if ( !graphic.isNull() )
        {
            QVariant iconValue;
            qVariantSetValue( iconValue, graphic );
            data.setValue( QwtLegendData::IconRole, iconValue );
        }

        QList<QwtLegendData> list;
        list += data;

        legend->updateLegend(curve, list);
    }
}

void LineGraphWidget::setZoomBase()
{
    auto b_rect = boundingRect();
    if (b_rect != rectangleZoom->zoomBase())
    {
        auto xIntv = axisInterval(QwtPlot::xBottom);
        auto yIntv = axisInterval(QwtPlot::yLeft);

        rectangleZoom->setZoomBase(b_rect.normalized());

        // restore axis intervals
        setAxisIntv(xIntv, QwtPlot::xBottom);
        setAxisIntv(yIntv, QwtPlot::yLeft);
    }
}

/*!
 * \brief LineGraphWidget::setLabel creates labels from \a annotation in form of multiple AClassRectItem.
 * \param xpos
 * \param annotation
 * \a isUserAnnotation is used to determine wether a user defined or detected label should be created.
 * For class only annotations with n classes n AClassRectItem stacked on top of each other with uniform sizes are created.
 * For numeric annotations the size of each ractangle is based on their value relative to the sum of all values.
 */
void LineGraphWidget::setLabel(uint timestamp, Annotation annotation, bool isUserAnnotation)
{
    // init labelMap dependent on isUserAnnotation
    QMap<uint, QList<AClassRectItem *>>* labelMap;
    if (isUserAnnotation)
        labelMap = &userDefinedClassLabels;
    else
        labelMap = &detectedClassLabels;

    // remove old label
    if (labelMap->contains(timestamp))
        deleteLabel(timestamp, isUserAnnotation);

    // draw new label
    if (!annotation.isEmpty())
    {
        QList<aClass> classList = annotation.getClasses();

        // remove numeric classes with value == 0.0 from classList
        // delete labels with value == 0.0
        for (int i=0; i<classList.size(); i++)
        {
            if (classList[i].getType() == aClass::Type::NUMERIC &&  classList[i].getValue() <= LGW_DET_CLASS_TRESH)
            {
                classList.removeAt(i);
                i--;    // decrement i in order to keep i the same value for the next turn
            }
        }

        // create new labels
        QList<AClassRectItem*> labels;
        Annotation drawAnnotation = Annotation ( classList.toSet() );
        for (aClass aclass : classList)
        {
            auto b_rect = boundingRect();
            auto xIntv = QwtInterval(getT(timestamp - 1), getT(timestamp + 1));
            auto classRect = new AClassRectItem(xIntv, drawAnnotation, aclass, isUserAnnotation);
            classRect->attach(this);
            labels <<  classRect;

        }
        (*labelMap)[timestamp] = labels;
    }
}

void LineGraphWidget::adjustLabels (bool isUserAnnotation)
{
    // init labelMap dependent on isUserAnnotation
    QMap<uint, QList<AClassRectItem *>>* labelMap;
    if (isUserAnnotation)
        labelMap = &userDefinedClassLabels;
    else
        labelMap = &detectedClassLabels;

    // adjust width of labels
    uint prevTimestamp = 0;
    for ( uint timestamp : labelMap->keys() )
    {
        long deltaT = timestamp - prevTimestamp;
        if (deltaT == 1 || deltaT == 3)
        {
            for (auto rect : (*labelMap)[prevTimestamp])
                rect->setRight(getT(prevTimestamp + 0.5 * deltaT));
            for (auto rect : (*labelMap)[timestamp])
                rect->setLeft(getT(prevTimestamp + 0.5 * deltaT));
        }
    }
}

void LineGraphWidget::deleteLabel(double t, bool isUserAnnotation)
{
    QMap<uint, QList<AClassRectItem *>>* labelMap;
    if (isUserAnnotation)
        labelMap = &userDefinedClassLabels;
    else
        labelMap = &detectedClassLabels;

    Q_ASSERT(labelMap->contains(t));

    for (auto rect : (*labelMap)[t])
        delete rect;
    labelMap->remove(t);
}


void LineGraphWidget::setMeasRunning(bool value)
{
    measRunning = value;
}

void LineGraphWidget::setReplotStatus(bool value)
{
    if (value != replotStatus)
    {
        replotStatus = value;

        if (value)
        {
            adjustLabels(true);
            adjustLabels(false);
            setZoomBase();
            replot();
        }
    }
}

QPair<double, double> LineGraphWidget::getSelectionRange()
{
    auto intv = zoneItem->interval();
    return QPair<double, double>(intv.minValue(), intv.maxValue());
}

void LineGraphWidget::initPlot(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    Q_ASSERT(dataCurves.size() == 0);

    // add graphs
    for (size_t i=0; i<vector.getSize(); i++)
    {
        // add data graph
        QString graphName = getGraphName(i, functionalisation);
        QColor graphColor = getGraphColor(i, functionalisation);

        QwtPlotCurve* curve = new QwtPlotCurve(graphName);

        curve->setPen(graphColor);
        curve->setStyle( QwtPlotCurve::CurveStyle::Lines );
        curve->setSymbol( new QwtSymbol( QwtSymbol::Ellipse,
                                           QBrush(graphColor), QPen(graphColor), QSize( 4, 4 ) ) );

//        curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

        curve->setData( new CurveData() );
        curve->attach( this );

        dataCurves << curve;

        // add selection graph
        QwtPlotCurve* selectionCurve = new QwtPlotCurve(graphName);

        selectionCurve->setPen(graphColor);
        selectionCurve->setStyle( QwtPlotCurve::CurveStyle::Lines );
        selectionCurve->setSymbol( new QwtSymbol( QwtSymbol::Ellipse,
                                           QBrush(graphColor), QPen(graphColor), QSize(6, 6)));


        selectionCurve->setData(new CurveData());
        selectionCurve->attach(this);

        selectionCurves << selectionCurve;
    }

    QDateTime datetime = QDateTime::fromTime_t(timestamp);
    setPrevXRange(datetime.addSecs(qRound(0.9 * LGW_AUTO_MOVE_ZONE_SIZE)), LGW_AUTO_MOVE_ZONE_SIZE);

    setupLegend(functionalisation, sensorFailures);

    if (replotStatus)
        replot();
}

QString LineGraphWidget::getGraphName(size_t i, const Functionalisation &functionalisation)
{
    return "ch" + QString::number(i+1);
}

QColor LineGraphWidget::getGraphColor(uint i, const Functionalisation &functionalisation)
{
    return ENoseColor::instance().getFuncColor(functionalisation[i]);
}

void LineGraphWidget::addPoint(QwtPlotCurve *curve, QPointF point)
{
    CurveData *curveData = static_cast<CurveData *>( curve->data() );

    // check max y
    if (point.y() > DBL_MAX)
        point.setY(DBL_MAX / 2);

    curveData->append( point );
}

void LineGraphWidget::addVector(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    Q_ASSERT(dataCurves.size() == 0 || vector.getSize() == dataCurves.size());

    // graph empty:
    // init graph
    if (dataCurves.size() == 0)
        initPlot(timestamp, vector, functionalisation, sensorFailures);

    auto datetime = QDateTime::fromTime_t(timestamp);
    double t = getT(timestamp);

    for (int i=0; i<dataCurves.size(); i++)
    {
        QwtPlotCurve* curve = dataCurves[i];
        QPointF point(t, vector[i]);

        addPoint(curve, point);
    }

    // set the zoomBase updated by addPoint
    if (replotStatus)
        setZoomBase();

    // auto-move x axis
    bool autoMoved = autoMoveXRange(t);

    // add annotation labels of vector
    if ( !vector.userAnnotation.isEmpty() )
    {
        setLabel(timestamp, vector.userAnnotation, true);
        if (replotStatus)
            adjustLabels(true);
    }
    if ( !vector.detectedAnnotation.isEmpty() )
    {
        setLabel(timestamp, vector.detectedAnnotation, false);
        if (replotStatus)
            adjustLabels(false);
    }

    if (replotStatus)
    {
        auto xIntv = axisInterval(QwtPlot::xBottom);
        if (qFuzzyCompare( xIntv.width(), LGW_AUTO_MOVE_ZONE_SIZE*1000 ) && xIntv.contains(t) )
            autoScale(false, true);
        replot();
    }
}

RelativeLineGraphWidget::RelativeLineGraphWidget(QWidget* parent):
    LineGraphWidget(parent)
{
    setAxisTitle(QwtPlot::yLeft, QString(u8"\u0394") + "R / R0 [%]");
}

void RelativeLineGraphWidget::addVector(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    Q_ASSERT(dataCurves.size() == 0 || functionalisation.size() == dataCurves.size());
    Q_ASSERT(dataCurves.size() == 0 || sensorFailures.size() == dataCurves.size());

    LineGraphWidget::addVector(timestamp, vector, functionalisation, sensorFailures);
}

QRectF RelativeLineGraphWidget::boundingRect() const
{
    QRectF rect = LineGraphWidget::boundingRect();

    // add relative margins
    double deltaX = LGW_X_RELATIVE_MARGIN * rect.width();
    double deltaY = LGW_Y_RELATIVE_MARGIN * rect.height();

    rect.adjust(-deltaX, -0.3*deltaY, deltaX, 2*deltaY);

    return rect;
}

void AbsoluteLineGraphWidget::initPlot(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    LineGraphWidget::initPlot(timestamp, vector, functionalisation, sensorFailures);
    setSensorFailures(sensorFailures, functionalisation);
}

AbsoluteLineGraphWidget::AbsoluteLineGraphWidget(QWidget* parent):
    LineGraphWidget(parent)
{
    setAxisTitle(QwtPlot::yLeft, "R [k" + QString(u8"\u2126") + "]");
}

void AbsoluteLineGraphWidget::addVector(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    Q_ASSERT(dataCurves.size() == 0 || functionalisation.size() == dataCurves.size());
    Q_ASSERT(dataCurves.size() == 0 || sensorFailures.size() == dataCurves.size());

    LineGraphWidget::addVector(timestamp, vector / 1000., functionalisation, sensorFailures);   // add vector / kOhm
}

QRectF AbsoluteLineGraphWidget::boundingRect() const
{
    QRectF rect = LineGraphWidget::boundingRect();

    // add relative margins
    double deltaX = LGW_X_RELATIVE_MARGIN * rect.width();
    double deltaY = LGW_Y_RELATIVE_MARGIN * rect.height();

    rect.adjust(-deltaX, -deltaY, deltaX, 2*deltaY);

    return rect;
}

void RelativeLineGraphWidget::initPlot(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    LineGraphWidget::initPlot(timestamp, vector, functionalisation, sensorFailures);
    setSensorFailures(sensorFailures, functionalisation);
}

FuncLineGraphWidget::FuncLineGraphWidget(QWidget* parent):
    LineGraphWidget(parent)
{
    setAxisTitle(QwtPlot::yLeft, QString(u8"\u0394") + "R / R0 [%]");
}

QRectF FuncLineGraphWidget::boundingRect() const
{
    QRectF rect = LineGraphWidget::boundingRect();

    // add relative margins
    double deltaX = LGW_X_RELATIVE_MARGIN * rect.width();
    double deltaY = LGW_Y_RELATIVE_MARGIN * rect.height();

    rect.adjust(-deltaX, -0.3*deltaY, deltaX, 2*deltaY);

    return rect;
}

void FuncLineGraphWidget::addVector(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    Q_ASSERT(dataCurves.size() == 0 || vector.getSize() == functionalisation.getNFuncs());

    LineGraphWidget::addVector(timestamp, vector, functionalisation, sensorFailures);

    setSensorFailures(sensorFailures, functionalisation);
}

void FuncLineGraphWidget::setSensorFailures(const std::vector<bool> &sensorFailures, const Functionalisation &functionalisation)
{
    auto funcMap = functionalisation.getFuncMap(sensorFailures);
    for (int i=0; i<funcMap.size(); i++)
    {
        if (funcMap.values()[i] == 0)
        {
            dataCurves[i]->setVisible(false);
            selectionCurves[i]->setVisible(false);
        }
    }

    if (replotStatus)
    {
        setupLegend(functionalisation, sensorFailures);
        replot();
        setZoomBase();
    }
}

void FuncLineGraphWidget::setFunctionalisation(const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    throw std::runtime_error("Do not call setFunctionalisation of a FuncLineGraphWidget! Instead, the graph has to be redrawn with the recalculated func vectors!");
}

QString FuncLineGraphWidget::getGraphName(size_t i, const Functionalisation &functionalisation)
{
    auto funcMap = functionalisation.getFuncMap(std::vector<bool>(functionalisation.size(), false));
    return "f" + QString::number(funcMap.keys()[i]);
}

QColor FuncLineGraphWidget::getGraphColor(uint i, const Functionalisation &functionalisation)
{
    return ENoseColor::instance().getFuncColor(functionalisation.getFuncMap().keys()[i]);
}

void FuncLineGraphWidget::setupLegend(const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures)
{
    auto funcMap = functionalisation.getFuncMap(sensorFailures);

    for (int i=0; i<dataCurves.size(); i++)
    {
        if (funcMap.values()[i] == 0)
            dataCurves[i]->setItemAttribute(QwtPlotItem::Legend, false);

        // hide selection curves from legend
        selectionCurves[i]->setItemAttribute(QwtPlotItem::Legend, false);
    }

    updateLegend();
}
