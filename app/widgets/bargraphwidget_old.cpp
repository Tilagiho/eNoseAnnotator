#include "bargraphwidget_old.h"
#include "ui_bargraphwidget.h"

#include "../classes/enosecolor.h"
#include "../classes/measurementdata.h"

BarGraphWidgetOld::BarGraphWidgetOld(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BarGraphWidget)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentWidget(ui->funcBarGraph);
    connect(ui->barGraph, &QCustomPlot::mousePress, this, &BarGraphWidgetOld::mousePressed);
    connect(ui->funcBarGraph, &QCustomPlot::mousePress, this, &BarGraphWidgetOld::mousePressed);

    initGraph();
}

BarGraphWidgetOld::~BarGraphWidgetOld()
{
    delete ui;
}

void BarGraphWidgetOld::initGraph()
{
    // set light background gradient:
    ui->barGraph->setBackground(QBrush(QColor(255,250,240)));
    ui->funcBarGraph->setBackground(QBrush(QColor(255,250,240)));


    // init bars & ticks
    QVector<double> ticks;

    for (uint i=0; i<MVector::nChannels; i++)
    {
        // init bar graphs
        sensorBarVector << new QCPBars(ui->barGraph->xAxis, ui->barGraph->yAxis);
        sensorBarVector[i]->setAntialiased(false);

        // set color
        QColor color = ENoseColor::instance().getSensorColor(i);
        sensorBarVector[i]->setPen(QPen(color.lighter(170)));
        sensorBarVector[i]->setBrush(color);

        // add tick + label
        ticks.append(i+1);
    }

    QVector<double> data;
    // set data:
    // i-th element of i-th bar is initialized with vector[i]
    for (int i=0; i<MVector::nChannels; i++)
    {
        if (!data.isEmpty())
            data.clear();

        for (int j=0; j<MVector::nChannels; j++)
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
    ui->barGraph->yAxis->setLabel(QString(u8"\u0394") + " R / R0 [%]");

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
    ui->funcBarGraph->yAxis->setLabel(QString(u8"\u0394") + " R / R0 [%]");

    resetColors();
}

void BarGraphWidgetOld::replot()
{
    // replot both graphs
    ui->barGraph->yAxis->rescale();
    ui->barGraph->replot();

    ui->funcBarGraph->xAxis->setRange(-1, funcBarVector.size());
    ui->funcBarGraph->yAxis->rescale();
    ui->funcBarGraph->replot();

    resetColors();
}

void BarGraphWidgetOld::setInputFunctionType(const InputFunctionType &value)
{
    inputFunctionType = value;
}

void BarGraphWidgetOld::setBars(MVector new_vector, std::vector<bool> sensorFailures, std::vector<int> functionalisation)
{
    if (mode == Mode::showAll)
    {
        // update fullBarVector
        QVector<double> ticks;
        for (int i=0; i<MVector::nChannels; i++)
            ticks << i+1;

        QMap<int, QVector<double>> data;

        for (int i=0; i<MVector::nChannels; i++)
        {

            for (int j=0; j<MVector::nChannels; j++)
            {
                if (i==j && !sensorFailures[i])
                    data[i] << new_vector[i];
                else
                    data[i] << 0.0;
            }
        }

        for (int i=0; i<MVector::nChannels; i++)
            sensorBarVector[i]->setData(ticks, data[i]);
    }
    else if (mode == Mode::showFunc)
    {
        // update funcBarVector:
        // get number of functionalisations, ignore sensor failures
        QMap<int, int> funcMap = MeasurementData::getFuncMap(functionalisation, sensorFailures);

        auto keyList = funcMap.keys();
        int funcSize = funcMap.size();
        // no func set: plot channels
        if (funcSize == 1)
            funcSize = MVector::nChannels;

        // new number of functionalisations:
        // reinit funcBarVector
        // ignore funcSize == 0 (no functionalisation set)
        if (funcSize != funcBarVector.size())
        {
            deleteBars();

            for (int i=0; i<funcSize; i++)
            {
                // init bar
                funcBarVector << new QCPBars(ui->funcBarGraph->xAxis, ui->funcBarGraph->yAxis);
                funcBarVector[i]->setAntialiased(false);

                // set color
                QColor color;
                if (funcSize == MVector::nChannels)
                    color = ENoseColor::instance().getSensorColor(i);
                else
                    color = ENoseColor::instance().getFuncColor(i);
                funcBarVector[i]->setPen(QPen(color.lighter(170)));
                funcBarVector[i]->setBrush(color);
            }
        }

        // set funcBarVector data
        // update fullBarVector
        QVector<double> funcTicks;
        bool isFuncBarGraph = funcMap.size() > 1;
        for (int i=0; i<funcSize; i++)
        {
            if (isFuncBarGraph)
                funcTicks << funcMap.keys()[i];
            else
                funcTicks << i;
        }

        // get func vector
        MVector funcVector = new_vector.getFuncVector(functionalisation, sensorFailures, inputFunctionType);

        // assign funcVector values to funcData
        QMap<int, QVector<double>> funcDataMap;
        for (int i=0; i<funcSize; i++)
            for (int j=0;j<funcSize; j++)
                if (i==j)
                    funcDataMap [i] << funcVector[i];
                else
                    funcDataMap [i] << 0.0;

        // set funcData
        for (int i=0; i<funcSize; i++)
            funcBarVector[i]->setData(funcTicks, funcDataMap[i]);
    }
    else
        Q_ASSERT("Unknown bar graph mode!" && false);

    replot();
}

void BarGraphWidgetOld::clearBars()
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

    // replot
    ui->barGraph->yAxis->rescale();
    ui->barGraph->replot();

    ui->funcBarGraph->yAxis->rescale();
    ui->funcBarGraph->replot();
}

void BarGraphWidgetOld::deleteBars()
{
    while (ui->barGraph->plottableCount() > 0)
        ui->barGraph->removePlottable(ui->barGraph->plottable(0));

    sensorBarVector.clear();
    funcBarVector.clear();
}

BarGraphWidgetOld::Mode BarGraphWidgetOld::getMode() const
{
    return mode;
}

void BarGraphWidgetOld::setMode(const Mode &value)
{
    mode = value;

    if (mode == Mode::showAll)
        ui->stackedWidget->setCurrentWidget(ui->barGraph);
    else
        ui->stackedWidget->setCurrentWidget(ui->funcBarGraph);
}

void BarGraphWidgetOld::mousePressed(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        QCustomPlot* plot = static_cast<QCustomPlot*> (ui->stackedWidget->currentWidget());

        QMenu *menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);

        menu->addAction("Save Graph...", this, [this](){
            emit imageSaveRequested();
        });

        menu->popup(plot->mapToGlobal(event->pos()));
    }
}

bool BarGraphWidgetOld::saveImage(const QString &filename)
{
    QStringList splitFilename = filename.split(".");
    Q_ASSERT("No file extension set!" && splitFilename.size() > 1);

    bool writeOk;
    QString extension = splitFilename.last();
    QCustomPlot* plot = static_cast<QCustomPlot*> (ui->stackedWidget->currentWidget());

    if (extension == "pdf")
        writeOk = plot->savePdf(filename);
    else if (extension == "bmp")
        writeOk = plot->saveBmp(filename);
    else if (extension == "jpg" || extension == "jpeg")
        writeOk = plot->saveJpg(filename);
    else if (extension == "png")
        writeOk = plot->savePng(filename);
    else
        Q_ASSERT("Unknown file extension!" && false);

    return writeOk;
}

void BarGraphWidgetOld::resetColors()
{
    if (mode == Mode::showAll)
    {
        for (int i=0; i<sensorBarVector.size(); i++)
        {
            QColor color = ENoseColor::instance().getSensorColor(i);
            QPen pen;
            pen.setColor(color.lighter(170));

            sensorBarVector[i]->setPen(pen);
            sensorBarVector[i]->setBrush(color);
        }
    }
    else    // func vectors
    {
        for (int i=0; i<funcBarVector.size(); i++)
        {
            QColor color;
            if (funcBarVector.size() == MVector::nChannels)
                color = ENoseColor::instance().getSensorColor(i);
            else
                color = ENoseColor::instance().getFuncColor(i);
            QPen pen;
            pen.setColor(color.lighter(170));

            funcBarVector[i]->setPen(pen);
            funcBarVector[i]->setBrush(color);
        }
    }

    ui->barGraph->replot();
}

void BarGraphWidgetOld::resetNChannels()
{
    clearBars();
    deleteBars();
    initGraph();
}
