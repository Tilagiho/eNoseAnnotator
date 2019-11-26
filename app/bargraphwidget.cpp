#include "bargraphwidget.h"
#include "ui_bargraphwidget.h"
#include "sensorcolor.h"

BarGraphWidget::BarGraphWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BarGraphWidget)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentWidget(ui->funcBarGraph);
    initGraph();
}

BarGraphWidget::~BarGraphWidget()
{
    delete ui;
}

void BarGraphWidget::initGraph()
{
    // set light background gradient:
    ui->barGraph->setBackground(QBrush(QColor(255,250,240)));
    ui->funcBarGraph->setBackground(QBrush(QColor(255,250,240)));


    // init bars & ticks
    QVector<double> ticks;

    for (uint i=0; i<MVector::size; i++)
    {
        // init bar graphs
        sensorBarVector << new QCPBars(ui->barGraph->xAxis, ui->barGraph->yAxis);
        sensorBarVector[i]->setAntialiased(false);

        // set color
        QColor color = SensorColor::getSensorColor(i);
        sensorBarVector[i]->setPen(QPen(color.lighter(170)));
        sensorBarVector[i]->setBrush(color);

        // add tick + label
        ticks.append(i+1);
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

        sensorBarVector[i]->setData(ticks, data);
    }

    // prepare barGraph
    ui->barGraph->xAxis->setTickLabelRotation(60);
    ui->barGraph->xAxis->setSubTicks(false);
    ui->barGraph->xAxis->setTickLength(0, 1);
    ui->barGraph->xAxis->setRange(-1, 65);
    ui->barGraph->xAxis->setBasePen(QPen(Qt::black));
    ui->barGraph->xAxis->setTickPen(QPen(Qt::black));
    ui->barGraph->xAxis->grid()->setVisible(true);
    ui->barGraph->xAxis->grid()->setPen(QPen(Qt::black, 0, Qt::DotLine));
    ui->barGraph->xAxis->setTickLabelColor(Qt::black);
    ui->barGraph->xAxis->setLabelColor(Qt::black);
    ui->barGraph->xAxis->setLabel("Sensor channel");

    ui->barGraph->yAxis->setRange(-2, 2);
    ui->barGraph->yAxis->setPadding(5); // a bit more space to the left border
    ui->barGraph->yAxis->setBasePen(QPen(Qt::black));
    ui->barGraph->yAxis->setTickPen(QPen(Qt::black));
    ui->barGraph->yAxis->setSubTickPen(QPen(Qt::black));
    ui->barGraph->yAxis->grid()->setSubGridVisible(true);
    ui->barGraph->yAxis->setTickLabelColor(Qt::black);
    ui->barGraph->yAxis->setLabelColor(Qt::black);
    ui->barGraph->yAxis->grid()->setPen(QPen(Qt::black, 0, Qt::SolidLine));
    ui->barGraph->yAxis->grid()->setSubGridPen(QPen(Qt::black, 0, Qt::DotLine));
    ui->barGraph->yAxis->setLabel("Deviation to base vector / %");

    // prepare funcBarGraph
    ui->funcBarGraph->xAxis->setTickLabelRotation(60);
    ui->funcBarGraph->xAxis->setSubTicks(false);
    ui->funcBarGraph->xAxis->setTickLength(0, 1);
    ui->funcBarGraph->xAxis->setBasePen(QPen(Qt::black));
    ui->funcBarGraph->xAxis->setTickPen(QPen(Qt::black));
    ui->funcBarGraph->xAxis->grid()->setVisible(true);
    ui->funcBarGraph->xAxis->grid()->setPen(QPen(Qt::black, 0, Qt::DotLine));
    ui->funcBarGraph->xAxis->setTickLabelColor(Qt::black);
    ui->funcBarGraph->xAxis->setLabelColor(Qt::black);
    ui->funcBarGraph->xAxis->setLabel("Functionalisation");

    ui->funcBarGraph->yAxis->setRange(-2, 2);
    ui->funcBarGraph->yAxis->setPadding(5); // a bit more space to the left border
    ui->funcBarGraph->yAxis->setBasePen(QPen(Qt::black));
    ui->funcBarGraph->yAxis->setTickPen(QPen(Qt::black));
    ui->funcBarGraph->yAxis->setSubTickPen(QPen(Qt::black));
    ui->funcBarGraph->yAxis->grid()->setSubGridVisible(true);
    ui->funcBarGraph->yAxis->setTickLabelColor(Qt::black);
    ui->funcBarGraph->yAxis->setLabelColor(Qt::black);
    ui->funcBarGraph->yAxis->grid()->setPen(QPen(Qt::black, 0, Qt::SolidLine));
    ui->funcBarGraph->yAxis->grid()->setSubGridPen(QPen(Qt::black, 0, Qt::DotLine));
    ui->funcBarGraph->yAxis->setLabel("Deviation to base vector / %");
}

void BarGraphWidget::replot()
{
    // replot both graphs
    ui->barGraph->yAxis->rescale();
    ui->barGraph->replot();

    ui->funcBarGraph->xAxis->setRange(-1, funcBarVector.size());
    ui->funcBarGraph->yAxis->rescale();
    ui->funcBarGraph->replot();
}

void BarGraphWidget::setBars(MVector new_vector, std::array<bool, MVector::size> sensorFailures, std::array<int, MVector::size> functionalisation)
{
    // update fullBarVector
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
                data << new_vector[i];
            else
                data << 0.0;
        }

        sensorBarVector[i]->setData(ticks, data);
    }

    // update funcBarVector:
    // get number of functionalisations, ignore sensor failures
    QMap<int, int> funcMap;
    for (int i=0; i<MVector::size; i++)
    {
        if (!sensorFailures[i])
        {
            if (!funcMap.contains(functionalisation[i]))
            {
                funcMap[functionalisation[i]] = 1;
            }
            else
                funcMap[functionalisation[i]]++;
        }
    }

    int funcSize = funcMap.size();
//    if (totalChannels > 1)
//        funcSize = funcMap.size();
    if (funcSize <= 1)
        funcSize = funcBarVector.size();

    // new number of functionalisations:
    // reinit funcBarVector
    // ignore funcSize == 0 (no functionalisation set)
    if (funcSize != 0 && funcSize != funcBarVector.size())
    {
        funcBarVector.clear();

        for (int i=0; i<funcSize; i++)
        {
            // init bar
            funcBarVector << new QCPBars(ui->funcBarGraph->xAxis, ui->funcBarGraph->yAxis);
            funcBarVector[i]->setAntialiased(false);

            // set color
            QColor color = SensorColor::getFuncColor(i, funcSize);
            funcBarVector[i]->setPen(QPen(color.lighter(170)));
            funcBarVector[i]->setBrush(color);

            // add tick + label
            ticks.append(i+1);
        }
    }

    // set funcBarVector data
    // update fullBarVector
    auto funcKeys = funcMap.keys();

    QVector<double> funcTicks;
    for (int i=0; i<funcSize; i++)
        funcTicks << funcKeys[i];

    // init funcData
    QMap<int, QVector<double>> funcDataMap;
    for (int i=0; i<funcSize; i++)
        for (int j=0;j<funcSize; j++)
            funcDataMap [i] << 0.0;

    // calc average of functionalisations
    for (int i=0; i<MVector::size; i++)
    {
        if (!sensorFailures[i])
        {
            for (int j=0; j<funcSize; j++)
            {
                int func = functionalisation[i];
                if (func == j)
                    funcDataMap[j][j] += new_vector[i] / funcMap[j];
            }
        }
    }

    // set funcData
    for (int i=0; i<funcSize; i++)
        funcBarVector[i]->setData(funcTicks, funcDataMap[i]);

    replot();
}

void BarGraphWidget::clearBars()
{
    QVector<double> data;
    QVector<double> ticks;

    // fullBarVector
    for (int i=0; i<sensorBarVector.size(); i++)
    {
        data << 0.0;
        ticks << i+1;
    }
    for (int i=0; i<sensorBarVector.size(); i++)
        sensorBarVector[i]->setData(ticks, data);

    // funcBarVector
    data.clear();
    ticks.clear();
    for (int i=0; i<funcBarVector.size(); i++)
    {
        data << 0.0;
        ticks << i;
    }
    for (int i=0; i<funcBarVector.size(); i++)
        funcBarVector[i]->setData(ticks, data);

    replot();
}

BarGraphWidget::Mode BarGraphWidget::getMode() const
{
    return mode;
}

void BarGraphWidget::setMode(const Mode &value)
{
    mode = value;

    if (mode == Mode::showAll)
        ui->stackedWidget->setCurrentWidget(ui->barGraph);
    else
        ui->stackedWidget->setCurrentWidget(ui->funcBarGraph);
}
