#include "bargraphwidget.h"

#include "../classes/enosecolor.h"
#include "../classes/defaultSettings.h"

#include <qwt_column_symbol.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_grid.h>

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
    d_barChartItem (new BarChartItem)
{    
    setCanvasBackground(QBrush(GRAPH_BACKGROUND_COLOR));

    setAxisTitle( QwtPlot::yLeft, QString(u8"\u0394") + "R / R0 [%]" );

    d_barChartItem->setLayoutPolicy( QwtPlotBarChart::AutoAdjustSamples );
    d_barChartItem->setMargin( 3 );

    d_barChartItem->attach( this );

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setMajorPen(QPen(Qt::DotLine));
    grid->attach( this );

    axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating,true);
    axisScaleEngine(QwtPlot::yLeft)->setMargins(1., 1.);
}

void AbstractBarGraphWidget::setVector( const MVector &vector, const std::vector<bool> sensorFailures, const Functionalisation &functionalisation )
{
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
}

void AbstractBarGraphWidget::clear()
{
    // set bars to zero
    QVector<double> values;
    for (size_t i=0; i<d_barChartItem->data()->size(); i++)
        values += 0.;

    d_barChartItem->setSamples(values);
    replot();
}

RelVecBarGraphWidget::RelVecBarGraphWidget ( QWidget *parent ) :
    AbstractBarGraphWidget(parent)
{
    setAxisTitle( QwtPlot::xBottom, "Channel" );
    d_barChartItem->setSpacing( 7 );

    // zero init
    setVector(MVector(), std::vector<bool>(MVector::nChannels, false), Functionalisation(MVector::nChannels, 0));
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
    setVector(MVector(nullptr, 1), std::vector<bool>(1, false), Functionalisation(1, 0));

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
