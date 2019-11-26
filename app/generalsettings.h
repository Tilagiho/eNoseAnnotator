#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <QtCore>
#include <QDialog>

#include "bargraphwidget.h"

namespace Ui {
class GeneralSettings;
}

class GeneralSettings : public QDialog
{
    Q_OBJECT

public:
    explicit GeneralSettings(QWidget *parent = nullptr);
    ~GeneralSettings();



    double getMaxVal() const;
    void setMaxVal(double value);

    bool getSaveRawInput() const;
    void setSaveRawInput(bool value);

    double getMinVal() const;
    void setMinVal(double value);

    bool getUseLimits() const;
    void setUseLimits(bool value);

    bool getShowAbsGraph() const;
    void setShowAbsGraph(bool value);

    BarGraphWidget::Mode getBarGraphMode() const;
    void setBarGraphMode(const BarGraphWidget::Mode &value);

private slots:
    void on_buttonBox_accepted();


private:
    Ui::GeneralSettings *ui;
    double maxVal;
    double minVal;
    bool saveRawInput = false;
    bool useLimits = true;
    bool showAbsGraph = false;
    BarGraphWidget::Mode barGraphMode = BarGraphWidget::Mode::showFunc;
};

#endif // GENERALSETTINGS_H
