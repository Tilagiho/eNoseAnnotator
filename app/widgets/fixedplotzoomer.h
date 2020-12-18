#ifndef FIXEDPLOTZOOMER_H
#define FIXEDPLOTZOOMER_H

#include <QObject>
#include <qwt_plot_zoomer.h>

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

#endif // FIXEDPLOTZOOMER_H
