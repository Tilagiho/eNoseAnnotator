#include "bargraphwidget.h"
#include "ui_bargraphwidget.h"
#include "sensorcolor.h"

BarGraphWidget::BarGraphWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BarGraphWidget)
{
    ui->setupUi(this);

    initGraph();
}

BarGraphWidget::~BarGraphWidget()
{
    delete ui;
}

void BarGraphWidget::initGraph()
{
    // set dark background gradient:
    QLinearGradient gradient(0, 0, 0, 400);
    gradient.setColorAt(0, QColor(90, 90, 90));
    gradient.setColorAt(0.38, QColor(105, 105, 105));
    gradient.setColorAt(1, QColor(70, 70, 70));
    ui->barGraph->setBackground(QBrush(gradient));

    // init bars & ticks
    QVector<double> ticks;
    QVector<QString> labels;

    for (uint i=0; i<MVector::size; i++)
    {
        // init bar
        barVector << new QCPBars(ui->barGraph->xAxis, ui->barGraph->yAxis);
        barVector[i]->setAntialiased(false);

        // set color
        QColor color = SensorColor::getColor(i);
        barVector[i]->setPen(QPen(color.lighter(170)));
        barVector[i]->setBrush(color);

        // add tick + label
        ticks.append(i+1);
        labels.append(QString(i+1));
    }

    QVector<double> data;
    // set data:
    // i-th element of i-th bar is initialized with vector[i]
    for (int i=0; i<MVector::size; i++)
    {
        if (!data.isEmpty())
            data.clear();

        for (int j=0; j<64; j++)
        {
            if (i==j)
                data << 0.0;
            else
                data << 0.0;
        }

        barVector[i]->setData(ticks, data);
    }

    // add ticker
    // QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    // textTicker->addTicks(ticks, labels);

    // ui->barGraph->xAxis->setTicker(textTicker);
    ui->barGraph->xAxis->setTickLabelRotation(60);
    ui->barGraph->xAxis->setSubTicks(false);
    ui->barGraph->xAxis->setTickLength(0, 1);
    ui->barGraph->xAxis->setRange(-1, 65);
    ui->barGraph->xAxis->setBasePen(QPen(Qt::white));
    ui->barGraph->xAxis->setTickPen(QPen(Qt::white));
    ui->barGraph->xAxis->grid()->setVisible(true);
    ui->barGraph->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
    ui->barGraph->xAxis->setTickLabelColor(Qt::white);
    ui->barGraph->xAxis->setLabelColor(Qt::white);

    // prepare y axis:
    ui->barGraph->yAxis->setRange(-2, 2);
    ui->barGraph->yAxis->setPadding(5); // a bit more space to the left border
    //ui->barGraph->yAxis->setLabel("Deviation");
    ui->barGraph->yAxis->setBasePen(QPen(Qt::white));
    ui->barGraph->yAxis->setTickPen(QPen(Qt::white));
    ui->barGraph->yAxis->setSubTickPen(QPen(Qt::white));
    ui->barGraph->yAxis->grid()->setSubGridVisible(true);
    ui->barGraph->yAxis->setTickLabelColor(Qt::white);
    ui->barGraph->yAxis->setLabelColor(Qt::white);
    ui->barGraph->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
    ui->barGraph->yAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
}

void BarGraphWidget::replot()
{
    ui->barGraph->yAxis->rescale();
    ui->barGraph->replot();
}

void BarGraphWidget::setBars(MVector new_vector, std::array<bool, MVector::size> sensorFailures)
{
    QVector<double> ticks;
    for (int i=0; i<MVector::size; i++)
        ticks << i+1;

    QVector<double> data;

    for (int i=0; i<MVector::size; i++)
    {
        if (!data.isEmpty())
            data.clear();

        for (int j=0; j<64; j++)
        {
            if (i==j && !sensorFailures[i])
                data << new_vector.array[i];
            else
                data << 0.0;
        }

        barVector[i]->setData(ticks, data);
    }

    replot();
}

void BarGraphWidget::clearBars()
{
    MVector data;
    std::array<bool, MVector::size> failures;

    for (int i=0; i<MVector::size; i++)
    {
        data.array[i] = 0.0;
        failures[i] = false;
    }

    setBars(data, failures);
}
