#ifndef QWTPLOTMAGNIFIER_H
#define QWTPLOTMAGNIFIER_H

#include <qwt_plot_magnifier.h>

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

#endif // QWTPLOTMAGNIFIER_H
