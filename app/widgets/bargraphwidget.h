#ifndef BARGRAPHWIDGET_H
#define BARGRAPHWIDGET_H

#include <QtCore>
#include <QWidget>

#include "../classes/mvector.h"
#include "../qcustomplot/qcustomplot.h"


namespace Ui {
class BarGraphWidget;
}

class BarGraphWidget : public QWidget
{
    Q_OBJECT

public:
    enum class Mode {
        showAll,
        showFunc
    };

    explicit BarGraphWidget(QWidget *parent = nullptr);
    ~BarGraphWidget();

    Mode getMode() const;
    void setMode(const Mode &value);

public slots:
    void setBars(MVector, std::array<bool, MVector::nChannels> sensorFailures, std::array<int, MVector::nChannels> functionalisation);
    void clearBars();
    bool saveImage(const QString &filename);

signals:
    void imageSaveRequested();

private:
    Ui::BarGraphWidget *ui;
    QVector<QCPBars*> sensorBarVector;    // contains 64 bars; one for each sensor in order to set seperate colors for each sensor
    QVector<QCPBars*> funcBarVector;

    Mode mode = Mode::showFunc;


    void initGraph();
    void replot();

private slots:
    void mousePressed(QMouseEvent*);

};

#endif // BARGRAPHWIDGET_H
