#ifndef BARGRAPHWIDGET_H
#define BARGRAPHWIDGET_H

#include <QWidget>

#include "../classes/mvector.h"

#include <qwt_plot.h>
#include <qwt_plot_barchart.h>

class BarChartItem: public QwtPlotBarChart
{
public:
    BarChartItem();

    void setSamples( const QVector<double> &values, const QStringList &labels, const QList<QColor> &colors );
    void setSamples ( const QVector<double> &values );

    virtual QwtColumnSymbol *specialSymbol(
        int index, const QPointF& ) const override;

    virtual QwtText barTitle( int sampleIndex ) const override;

private:
    QList<QColor> d_colors;
    QList<QString> d_labels;
};

class AbstractBarGraphWidget : public QwtPlot
{
    Q_OBJECT
public:
    explicit AbstractBarGraphWidget(QWidget *parent = nullptr);

signals:
    void saveRequested();

public slots:
    void setVector(const MVector &vector, const std::vector<bool> sensorFailures, const Functionalisation &functionalisation);
    void clear();
    void exportGraph(QString filePath);

protected:
    BarChartItem *d_barChartItem;

    virtual void mouseReleaseEvent(QMouseEvent *event) override;

protected slots:
    virtual QColor getColor(uint channel, const Functionalisation &functionalisation) const = 0;
    virtual void setValues(const QVector<double> &values, const Functionalisation &functionalisation) = 0;
};

class RelVecBarGraphWidget : public AbstractBarGraphWidget
{
    Q_OBJECT
public:
    explicit RelVecBarGraphWidget ( QWidget *parent = nullptr );

public slots:

protected slots:
    QColor getColor(uint channel, const Functionalisation &functionalisation) const override;
    virtual void setValues(const QVector<double> &values, const Functionalisation &functionalisation) override;
};

class FuncBarGraphWidget : public AbstractBarGraphWidget
{
    Q_OBJECT
public:
    explicit FuncBarGraphWidget(QWidget *parent = nullptr);

public slots:

protected slots:
    QColor getColor(uint channel, const Functionalisation &functionalisation) const override;
    virtual void setValues(const QVector<double> &values, const Functionalisation &functionalisation) override;
};

#endif // BARGRAPHWIDGET_H
