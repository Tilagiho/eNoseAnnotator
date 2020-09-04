#ifndef LINEGRAPH_H
#define LINEGRAPH_H

#include <QtCore>
#include <QWidget>

#include "../classes/mvector.h"
#include "../qcustomplot/qcustomplot.h"

namespace Ui {
class LineGraphWidget;
}

class ReplotWorker: public QObject
{
    Q_OBJECT

public:
    ReplotWorker(QMutex* graphMutex, QObject *parent = nullptr):
        QObject(parent),
        graphMutex(graphMutex)
    {}

public Q_SLOTS:
    void replot(Ui::LineGraphWidget *ui);

Q_SIGNALS:
    void finished(double, double);
    void error(QString errorMessage);

private:
    const int replot_steps = 1;

    const double yMin = 2.5;    // defines minimum range of yAxis: (-yMin;yMin)
    const double labelSpace = 0.3;

    QMutex sync;
    QMutex *graphMutex;
};

class LineGraphWidget : public QWidget
{
    Q_OBJECT

public:


    explicit LineGraphWidget(QWidget *parent = nullptr, int nChannels = MVector::nChannels);
    ~LineGraphWidget();
    void clearGraph(bool replot = true);

    bool selectionEmpty();

    std::array<uint, 2> getSelection();  // returns start and end timestamp of selection
    void setStartTimestamp(uint timestamp);
    QPair<double, double> getXRange();
    void setXRange(QPair<double, double>);
    void setYRange(double lower, double upper);
    void setLogXAxis(bool logOn);
    void setMaxVal(double val);

    double getMaxVal() const;

    double getMinVal() const;
    void setMinVal(double value);

    // flag to determine if minVal and maxVal should be used when plotting data


    bool getUseLimits() const;
    void setUseLimits(bool value);


    void setIsAbsolute(bool value);

    int getNChannels() const;

    void resetGraph(int channels = MVector::nChannels);

    // static variables
    // interval in which data is plotted: data > maxVal or < minVal is ignored
    // can be ignored with useLimits
    static double maxVal;
    static double minVal;

    void setNChannels(int value);

public Q_SLOTS:
    void addMeasurement(MVector measurement, uint timestamp, bool rescale=true);   // add single measurement; rescale y-axis if rescale==true
//    void addMeasurement(QVector<MVector> measurements, QVector<uint> timestamps);   // add multiple measurements
//    void addMeasurement(QMap<uint, MVector>);
    void setData(QMap<uint, MVector> map);
    void setSensorFailureFlags(const std::vector<bool> sensorFailureFlags);
    void setAutoMoveGraph(bool value);
    void clearSelection();

    void setReplotStatus(bool value);

    void setSelection (QCPDataSelection selection);

    bool saveImage(const QString &filename);
    QPixmap getPixmap();

    /*
     * draws selection and class rectangles
     */
    void labelSelection(QMap<uint, MVector> selectionMap);

    void setXRange(QCPRange range);

    void resetColors();

signals:
    void selectionChanged(int, int);
    void dataSelectionChanged(QCPDataSelection);
    void selectionCleared();
    void sensorFailure(std::vector<bool>);
    void requestRedraw();
    void xRangeChanged(const QCPRange new_range);
    void ImageSaveRequested();

private:
    Ui::LineGraphWidget *ui;
    QThread *thread = nullptr;
    ReplotWorker* worker = nullptr;
    int nChannels = MVector::nChannels;

    QMutex* graphMutex;

    const double det_class_tresh = 0.001;   // only classes with values higher than this are shown in the detected class labels

    const int defaultXWidth = 30; // defines default range of xAxis: (-1; defaultXWidth)
    const double yMin = 2.5;    // defines minimum range of yAxis: (-yMin;yMin)
    const double labelSpace = 0.3;

    bool useLimits = false;
    bool isAbsolute = false;

    uint startTimestamp; // timestamp for start of graph
    QCPDataSelection dataSelection; // holds current data selection
    bool selectionFlag = false;     // used to avoid looping behaviour when selecting data
    bool replotStatus = true;   // replot() is only active if true

    std::vector<bool> sensorFailureFlags;
    bool autoMoveGraph = true;

    QMap<int, QPair<QString, QList<QCPItemRect *>>> userDefinedClassLabels;
    QMap<int, QPair<QString, QList<QCPItemRect *>>> detectedClassLabels;

    QCPItemText* coordText = nullptr;

    void setupGraph();

    /*
     * returns index of data point where x == key on chart
     * returns -1 if not found
     */
    double getIndex (int key);

private slots:
    void replot(uint timestamp=0);
    void mousePressed(QMouseEvent*);
    void mouseMoved	(QMouseEvent *  event);
    void resizeEvent(QResizeEvent*);

    void dataSelected();
    void onXRangeChanged(QCPRange range);

    /*
     * draws label of user+detected class at top of graph
     */
    void setLabel(int xpos, Annotation annotation, bool isUserAnnotation);

    /*
     *  goes through userDefinedLabels and detectedLabels in range of x-axis
     *  joins successive labels with matching classes
     */
    void redrawLabels();

    /*
     * returns y-coord based on current y-range
     * userDefinedLabel == true: for user defined labels
     * else: for detected labels
     */
    QPair<double, double> getLabelYCoords(bool userDefinedLabel);

    void adjustLabelBorders(int firstX, int secondX);

    QCPGraph* firstVisibleGraph();
};

#endif // LINEGRAPH_H
