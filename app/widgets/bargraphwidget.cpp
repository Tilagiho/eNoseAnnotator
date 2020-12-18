#include "bargraphwidget.h"

#include "../classes/enosecolor.h"
#include "../classes/defaultSettings.h"
#include "fixedplotmagnifier.h"

#include <qwt_column_symbol.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_renderer.h>
#include <qwt_interval_symbol.h>
#include <qwt_symbol.h>
#include <qwt_painter.h>
#include <qwt_plot_panner.h>

#include <QMenu>
#include <QMouseEvent>
#include <QGuiApplication>

class FullTicksScaleEngine: public QwtLinearScaleEngine
{
public:
    FullTicksScaleEngine():
        QwtLinearScaleEngine()
    {}

    QwtScaleDiv divideScale (double x1, double x2, int maxMajorSteps, int maxMinorSteps, double stepSize=0.0) const override
    {
        int lowerBound = qRound(x1);
        int upperBound = qRound(x2);
        QList<double> minorTicks, mediumTicks, majorTicks;
        for (int i = lowerBound; i<=upperBound; i++)
            majorTicks << i;

        auto scaleDiv = QwtScaleDiv (x1, x2, minorTicks, mediumTicks, majorTicks);

        if ( x1 > x2 )
            scaleDiv.invert();

        return scaleDiv;
    }
};

class LabelScaleDraw: public QwtScaleDraw
{
public:
    LabelScaleDraw( Qt::Orientation orientation, const QStringList &labels ):
        d_labels( labels )
    {
        setTickLength( QwtScaleDiv::MinorTick, 0 );
        setTickLength( QwtScaleDiv::MediumTick, 0 );
        setTickLength( QwtScaleDiv::MajorTick, 1 );

        enableComponent( QwtScaleDraw::Backbone, false );

        if ( orientation == Qt::Vertical )
        {
            setLabelRotation( 0.0 );
        }
        else
        {
            setLabelRotation( -20.0 );
        }

        setLabelAlignment( Qt::AlignCenter );
    }

    virtual QwtText label( double value ) const override
    {
        QwtText lbl;

        const int index = qRound( value );
        if ( index >= 0 && index < d_labels.size() )
        {
            lbl = d_labels[ index ];
        }

        return lbl;
    }

private:
    const QStringList d_labels;
};


ErrorBarMarker::ErrorBarMarker (int index, double value, double error):
    index(index),
    value(value),
    error(error)
{}

void ErrorBarMarker::draw(QPainter *pPainter, const QwtScaleMap &pXMap, const QwtScaleMap &pYMap, const QRectF &pBoundingRectangle) const
{
    // ignore failing channels
    if (failure)
        return;

    // get pixel values for lines
    const int vLineX = static_cast<int>(pXMap.transform(index));
    const int hLineX1 = static_cast<int>(pXMap.transform(index-width));
    const int hLineX2 = static_cast<int>(pXMap.transform(index+width));
    const int yBottom = static_cast<int>(pYMap.transform(value - error));
    const int yTop = static_cast<int>(pYMap.transform(value + error));

    // select line pen
    pPainter->setPen(linePen());

    // draw bar chart symbol:
    // vertical line
    QwtPainter::drawLine(pPainter, vLineX, yBottom, vLineX, yTop);

    // horizontal line
    QwtPainter::drawLine(pPainter, hLineX1, yBottom, hLineX2, yBottom);
    QwtPainter::drawLine(pPainter, hLineX1, yTop, hLineX2, yTop);

    // Call the parent implementation for the other part of the QwtPlotMarker
    QwtPlotMarker::draw(pPainter,pXMap,pYMap,pBoundingRectangle);
}

void ErrorBarMarker::setFailure(bool value)
{
    failure = value;
}

BarChartItem::BarChartItem():
    QwtPlotBarChart( "Page Hits" )
{
    setLayoutPolicy( AutoAdjustSamples );
    setLayoutHint( 4.0 ); // minimum width for a single bar


//    setSpacing( 5 ); // spacing between bars
}

void BarChartItem::setSamples( const QVector<double> &values, const QStringList &labels, const QList<QColor> &colors )
{
    setSamples(values);
    d_colors = colors;
    d_labels = labels;
    itemChanged();
}

void BarChartItem::setSamples(const QVector<double> &values)
{
    // set bars
    QwtPlotBarChart::setSamples(values);
}

QwtColumnSymbol* BarChartItem::specialSymbol(
    int index, const QPointF& ) const
{
    // we want to have individual colors for each bar

    QwtColumnSymbol *symbol = new QwtColumnSymbol( QwtColumnSymbol::Box );
    symbol->setLineWidth( 2 );
    symbol->setFrameStyle( QwtColumnSymbol::FrameStyle::Raised );

    QColor c( Qt::white );
    if ( index >= 0 && index < d_colors.size() )
        c = d_colors[ index ];

    symbol->setPalette( c );

    return symbol;
}

QwtText BarChartItem::barTitle( int sampleIndex ) const
{
    QwtText title;
    if ( sampleIndex >= 0 && sampleIndex < d_labels.size() )
        title = d_labels[ sampleIndex ];

    return title;
}

AbstractBarGraphWidget::AbstractBarGraphWidget( QWidget *parent ) :
    QwtPlot(parent),
    d_barChartItem (new BarChartItem),
    rectangleZoom(new FixedPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, canvas()))
{
    setCanvasBackground(QBrush(GRAPH_BACKGROUND_COLOR));

    setAxisTitle( QwtPlot::yLeft, QString(u8"\u0394") + "R / R0 [%]" );

    // bars
    d_barChartItem->setLayoutPolicy( QwtPlotBarChart::AutoAdjustSamples );
    d_barChartItem->setMargin( 3 );

    d_barChartItem->attach( this );

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setMajorPen(QPen(Qt::DotLine));
    grid->attach( this );

    axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating,true);
    axisScaleEngine(QwtPlot::yLeft)->setMargins(1., 1.);

    // panner: drag and drop the graph range
    auto panner = new QwtPlotPanner(canvas());
    panner->setMouseButton(Qt::LeftButton, Qt::NoModifier);

    // mouse wheel zoom
    QwtPlotMagnifier *mouseWheelZoom = new FixedPlotMagnifier(canvas());
    mouseWheelZoom->setMouseButton(Qt::MouseButton::NoButton);
    mouseWheelZoom->setAxisEnabled(QwtPlot::yLeft, true);
    mouseWheelZoom->setAxisEnabled(QwtPlot::xBottom, false);

    // zoom rect -> Shift + Rightclick to show full vector
    rectangleZoom->setRubberBand( QwtPicker::RectRubberBand );
    rectangleZoom->setTrackerMode(QwtPicker::AlwaysOff);

    rectangleZoom->setMousePattern(QwtEventPattern::MouseSelect1,Qt::LeftButton, Qt::ShiftModifier);
    rectangleZoom->setMousePattern( QwtEventPattern::MouseSelect2,Qt::RightButton, Qt::ShiftModifier); //zoom out by 1
    rectangleZoom->setMousePattern( QwtEventPattern::MouseSelect3,Qt::MiddleButton, Qt::ShiftModifier); //zoom out by 1
}

QRectF AbstractBarGraphWidget::boundingRect() const
{
    return d_barChartItem->boundingRect();
;
}

void AbstractBarGraphWidget::setVector( const MVector &vector, const MVector &stdDevVector, const std::vector<bool> sensorFailures, const Functionalisation &functionalisation )
{
    // add bars
    bool useFailures = vector.getSize() == sensorFailures.size();

    QVector<double> values;
    for ( int i = 0; i < vector.getSize(); i++ )
    {
        if (useFailures && sensorFailures[i])
            values += 0.;
        else
            values += vector[i];
    }

    setValues(values, functionalisation);

    // add error bars:
    // clear previous errorBars
    for (auto errorBar : errorBars)
        delete errorBar;
    errorBars.clear();

    // add new error bars
    for (int i=0; i<stdDevVector.getSize(); i++)
    {
        ErrorBarMarker *errorbar = new ErrorBarMarker(i, vector[i], stdDevVector[i]);

        errorbar->setVisible(errorBarsVisible);
        if (sensorFailures[i])
                    errorbar->setFailure(true);

        errorbar->setSymbol(new QwtSymbol(QwtSymbol::Style::NoSymbol));
        errorbar->setLinePen(Qt::black, qPow(MVector::nChannels, 0.3) / qPow(stdDevVector.getSize(), 0.3));
        errorbar->attach(this);
        errorBars << errorbar;
    }

    // set zoom base
    // -> shift + rightclick to zooom to all bars
    setZoomBase();
}

void AbstractBarGraphWidget::clear()
{
    // set bars to zero
    QVector<double> values;
    for (size_t i=0; i<d_barChartItem->data()->size(); i++)
        values += 0.;

    d_barChartItem->setSamples(values);

    for (auto errorBar : errorBars)
        delete errorBar;
    errorBars.clear();

    replot();
}

void AbstractBarGraphWidget::exportGraph(QString filePath)
{
    int widthResolution = logicalDpiX();  //width dots per inch
    int heightResolution = logicalDpiY();  //width dots per inch

    double widthMM = 25.4 * static_cast<double>(size().width()) / static_cast<double>(widthResolution);
    double heightMM = 25.4 * static_cast<double>(size().height()) / static_cast<double>(heightResolution);
    QSizeF sizeMM(widthMM, heightMM);

    QwtPlotRenderer renderer;
    renderer.renderDocument(this, filePath, sizeMM, 150);
}

void AbstractBarGraphWidget::setErrorBarsVisible(bool value)
{
    if (value != errorBarsVisible)
    {
        errorBarsVisible = value;
        for (auto errorBar : errorBars)
            errorBar->setVisible(value);

        replot();

        emit errorBarsVisibleSet(value);
    }
}

void AbstractBarGraphWidget::setZoomBase()
{
    auto b_rect = boundingRect();
    // no vector set:
    // do nothing
    if (qFuzzyIsNull(b_rect.height()))
        return;

    if (b_rect != rectangleZoom->zoomBase())
    {
        auto xIntv = axisInterval(QwtPlot::xBottom);
        auto yIntv = axisInterval(QwtPlot::yLeft);

        rectangleZoom->setZoomBase(b_rect.normalized());

        // restore axis intervals
        if (!autoScale)
        {
            setAxisIntv(xIntv, QwtPlot::xBottom);
            setAxisIntv(yIntv, QwtPlot::yLeft);
        }
    }
}

void AbstractBarGraphWidget::setAxisIntv (QwtInterval intv, QwtPlot::Axis axis)
{
    auto currentIntv = axisInterval(axis);

    if (intv != currentIntv)
    {
        setAxisScale(axis, intv.minValue(), intv.maxValue());
        replot();
    }
}

void AbstractBarGraphWidget::setAutoScale(bool value)
{
    autoScale = value;

    // auto scale directly???
//    if (value)
//    {
//        auto rect = rectangleZoom->zoomBase();
//        QwtInterval xIntv(rect.left(), rect.right());
//        QwtInterval yIntv(rect.bottom(), rect.top());

//        setAxisIntv(xIntv.normalized(), QwtPlot::xBottom);
//        setAxisIntv(yIntv.normalized(), QwtPlot::yLeft);
//    }
}

void AbstractBarGraphWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton && !(QGuiApplication::keyboardModifiers() & Qt::ShiftModifier))
    {
        QMenu* menu = new QMenu(this);
        // auto scale
        QAction* autoScaleAction = new QAction("Auto scale bars", this);
        autoScaleAction->setCheckable(true);
        autoScaleAction->setChecked(autoScale);
        connect(autoScaleAction, &QAction::triggered, this, &AbstractBarGraphWidget::setAutoScale);
        menu->addAction(autoScaleAction);

        // error bars
        QAction* errorBarAction = new QAction("Show error bars", this);
        errorBarAction->setCheckable(true);
        errorBarAction->setChecked(errorBarsVisible);
        connect(errorBarAction, &QAction::triggered, this, &AbstractBarGraphWidget::setErrorBarsVisible);
        menu->addAction(errorBarAction);

        menu->addSeparator();

        // saving actions
        menu->addAction("Save graph as image...", this, &AbstractBarGraphWidget::saveRequested);

        menu->popup(this->mapToGlobal(event->pos()));
    }
}

RelVecBarGraphWidget::RelVecBarGraphWidget ( QWidget *parent ) :
    AbstractBarGraphWidget(parent)
{
    setAxisTitle( QwtPlot::xBottom, "Channel" );
    d_barChartItem->setSpacing( 7 );

    // zero init
    setVector(MVector(), MVector(), std::vector<bool>(MVector::nChannels, false), Functionalisation(MVector::nChannels, 0));
}

QColor RelVecBarGraphWidget::getColor( uint channel, const Functionalisation &functionalisation ) const
{
    return ENoseColor::instance().getFuncColor(functionalisation[channel]);

}

void RelVecBarGraphWidget::setValues(const QVector<double> &values, const Functionalisation &functionalisation)
{
    QStringList labels;
    QList<QColor> colors;
    for (int i=0; i<functionalisation.size(); i++)
    {
        labels << QString::number(i);
        colors << getColor(i, functionalisation);
    }
    setAxisScaleDraw( QwtPlot::xBottom, new LabelScaleDraw( Qt::Vertical, labels ) );
    d_barChartItem->setSamples(values, labels, colors);
    replot();
}

FuncBarGraphWidget::FuncBarGraphWidget( QWidget *parent ) :
    AbstractBarGraphWidget(parent)
{
    setAxisTitle( QwtPlot::xBottom, "Functionalisation" );
    d_barChartItem->setSpacing( 20 );

    // zero init
    setVector(MVector(nullptr, 1),MVector(nullptr, 1), std::vector<bool>(1, false), Functionalisation(1, 0));

//    setAxisScaleEngine(QwtPlot::xBottom, new FullTicksScaleEngine());
}

QColor FuncBarGraphWidget::getColor( uint channel, const Functionalisation &functionalisation ) const
{
    auto funcs = functionalisation.getFuncMap().keys();

    if (funcs.size() == 1)
        return ENoseColor::instance().getFuncColor( funcs.first() );
    else
        return ENoseColor::instance().getFuncColor( funcs[channel] );
}

void FuncBarGraphWidget::setValues(const QVector<double> &values, const Functionalisation &functionalisation)
{
    QStringList labels;
    QList<QColor> colors;
    auto funcList = functionalisation.getFuncMap().keys();
    for (int i=0; i<funcList.size(); i++)
    {
        labels << QString::number(funcList[i]);
        colors << getColor(i, functionalisation);
    }
    setAxisScaleDraw( QwtPlot::xBottom, new LabelScaleDraw( Qt::Vertical, labels ) );
    setAxisScaleEngine(QwtPlot::xBottom, new FullTicksScaleEngine);
    d_barChartItem->setSamples(values, labels, colors);
    replot();
}
