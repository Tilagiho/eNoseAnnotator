#ifndef LINEGRAPHWIDGET_H
#define LINEGRAPHWIDGET_H

#include <QWidget>

#include <qwt_plot.h>

#include "../classes/mvector.h"
#include "../classes/functionalisation.h"

#include <qwt_plot_zoomer.h>
#include <qwt_plot_picker.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoneitem.h>
#include <qwt_plot_textlabel.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_legenditem.h>

#define LGW_AUTO_MOVE_ZONE_SIZE 200
#define LGW_AUTO_MOVE_ZONE_BORDER_MIN 5
#define LGW_AUTO_MOVE_ZONE_BORDER_RATIO 0.15

#define LGW_X_RELATIVE_MARGIN 0.1
#define LGW_Y_RELATIVE_MARGIN 0.15

#define LGW_DET_CLASS_TRESH 0.01

class CurveData: public QwtArraySeriesData<QPointF>
{
public:
    CurveData();

    virtual QRectF boundingRect() const override;

    inline void append( const QPointF &point );

    void clear();

    QVector<QPointF>* samples();
};

class FixedPlotMagnifier: public QwtPlotMagnifier
{
    Q_OBJECT
public:
    explicit FixedPlotMagnifier( QWidget * parent):
        QwtPlotMagnifier(parent)
    {}

protected:
    virtual void rescale(double factor) override
    {
       if ( factor != 0.0 )
           QwtPlotMagnifier::rescale(1 / factor);
    }
};

/*!
 * \brief FixedPlotZoomer subclasses QwtPlotZoomer in order fix the zoom base:
 * - setZoomBase( const QRectF &base ) only sets the zoomBase based on base.
 */
class FixedPlotZoomer: public QwtPlotZoomer
{
    Q_OBJECT
public:
    explicit FixedPlotZoomer( int xAxis, int yAxis,
                              QWidget *parent, bool doReplot = true):
        QwtPlotZoomer(xAxis, yAxis, parent, doReplot)
    {}

    /*!
     * \brief setZoomBase sets base as the new zoomBase
     * (Original version unites base with scaleRect())
     */
    virtual void setZoomBase( const QRectF &base ) override
    {
        if(zoomBase() == base)
            return ;

        QStack<QRectF> stack = zoomStack() ; // get stack
        stack.remove(0) ; // remove old base
        stack.prepend(base) ; // add new base

        // put stack in place and try to find current zoomRect in it
        // (set index to current's index in new stack or top of stack
        // if the current rectangle is not in the new stack)
        setZoomStack(stack, -1) ;
     }
};

/*!
 * \brief The ToolTipPlotPicker class adds tooltips based on the graph items under/ close to the cursor
 */
class ToolTipPlotPicker: public QwtPlotPicker
{
    Q_OBJECT

public:
    explicit ToolTipPlotPicker(QWidget *parent):
        QwtPlotPicker(parent)
    {}

signals:
    void mouseMoved(const QPointF &pos) const;

protected:
    virtual QwtText trackerTextF (const QPointF &pos) const override;
};

class AClassRectItem: public QwtPlotItem
{
public:
    AClassRectItem(QwtInterval interval, Annotation annotation, aClass aclass, bool isUserAnnotation);

    void setInterval( QwtInterval& xValue );
    double left();
    void setLeft(const double &value);
    double right();
    void setRight(const double &value);

    virtual void draw( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect ) const override;

    QRectF boundingRect();

    int rtti() const override;

    Annotation getAnnotation() const;

    bool getIsUserAnnotation() const;

protected:
    QwtInterval xInterval;
    Annotation annotation;
    aClass aclass;
    bool isUserAnnotation;
    mutable QRectF b_rect;

    QColor getColor() const;
};

class LineGraphWidget : public QwtPlot
{
    Q_OBJECT
public:
    explicit LineGraphWidget(QWidget *parent = nullptr);
    ~LineGraphWidget();

    void setReplotStatus(bool value);

    QPair<double, double> getSelectionRange();

    void setMeasRunning(bool value);

    virtual QRectF boundingRect() const;

    double getT(double timestamp);

    double getT(uint timestamp);

    double getT(QDateTime datetime);

    uint getTimestamp(double t);

    void setAxisScale( int axisId, double min, double max, double stepSize = 0 );

    void exportGraph(QString filePath);

signals:
    void axisIntvSet(QwtInterval intv, QwtPlot::Axis axis);

    void selectionMade(uint min, uint max);

    void selectionCleared();

    void saveRequested();

public slots:
    virtual void addVector(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures);

    void clearGraph();

    void zoomToData();

    void autoScale(bool xAxis = true, bool yAxis = true);

    void setAxisIntv(QwtInterval intv, QwtPlot::Axis axis);

    void setPrevXRange(QDateTime datetime, int seconds);

    void shiftXRange(double seconds);

    void makeSelection(const QRectF &rect);

    void makeSelection(uint min, uint max);

    void makeSelection(double minT, double maxT);

    void clearSelection();

    virtual void setSensorFailures(const std::vector<bool> &sensorFailures, const Functionalisation &functionalisation);

    virtual void setFunctionalisation(const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures);

    void setAnnotations(const QMap<uint, Annotation> &annotations, bool isUserAnnotation);

    void adjustLabels ( bool isUserAnnotation );

protected slots:
    void setMouseCoordinates(const QPointF &coords);

    virtual void setupLegend(const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures);

//    void updateZoomBase(QPointF point);

    void setZoomBase();

    void setLabel(uint timestamp, Annotation annotation, bool isUserAnnotation);

    void deleteLabel(double t, bool isUserAnnotation);

protected:
    bool replotStatus = true;
    bool measRunning = false;

    QVector<QwtPlotCurve*> dataCurves;
    QVector<QwtPlotCurve*> selectionCurves;

    QMap<uint, QList<AClassRectItem *>> userDefinedClassLabels;
    QMap<uint, QList<AClassRectItem *>> detectedClassLabels;

    FixedPlotZoomer *rectangleZoom;
    QwtPlotPicker *zonePicker;
    QwtPlotZoneItem *zoneItem;
    ToolTipPlotPicker *toolTipPicker;

    QwtPlotTextLabel *coordinateLabel;
    QwtPlotLegendItem *legend;

    QPointF zoomBaseOffset = QPointF(2000., 1.);

    virtual void initPlot(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures);

    virtual QString getGraphName(size_t i, const Functionalisation &functionalisation);

    virtual QColor getGraphColor(uint i, const Functionalisation &functionalisation);

    void addPoint(QwtPlotCurve* curve, QPointF point);

    bool selectPoints(double min, double max);

    bool autoMoveXRange(double t);

    virtual void mouseReleaseEvent(QMouseEvent *event) override;

};

class AbsoluteLineGraphWidget : public LineGraphWidget
{
    Q_OBJECT
public:
    explicit AbsoluteLineGraphWidget(QWidget *parent = nullptr);

    void addVector(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures) override;

    virtual QRectF boundingRect() const override;

protected:
    virtual void initPlot(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures) override;
};

class RelativeLineGraphWidget : public LineGraphWidget
{
    Q_OBJECT

public:
    explicit RelativeLineGraphWidget(QWidget *parent = nullptr);

    void addVector(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures) override;

    virtual QRectF boundingRect() const override;

protected:
    virtual void initPlot(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures) override;
};

class FuncLineGraphWidget : public LineGraphWidget
{
    Q_OBJECT

public:
    explicit FuncLineGraphWidget(QWidget *parent = nullptr);

    virtual QRectF boundingRect() const override;

public slots:
    void addVector(uint timestamp, MVector vector, const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures) override;

    void setSensorFailures(const std::vector<bool> &sensorFailures, const Functionalisation &functionalisation) override;

    void setFunctionalisation(const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures) override;

    void setupLegend(const Functionalisation &functionalisation, const std::vector<bool> &sensorFailures) override;

protected:
    QString getGraphName(size_t i, const Functionalisation &functionalisation) override;
    QColor getGraphColor(uint i, const Functionalisation &functionalisation) override;
};

#endif // LINEGRAPHWIDGET_H
