#ifndef SENSORCOLOR_H
#define SENSORCOLOR_H

#include <vector>

#include <QtCore>
#include <QColor>

#include "annotation.h"

/*!
 * \brief The ENoseColor class implements a singleton based on https://stackoverflow.com/a/1008289
 */
class ENoseColor
{
public:
    //
    // singleton defenitions:
    //
    static ENoseColor& getInstance()
    {
        static ENoseColor instance; // Guaranteed to be destroyed.
                                    // Instantiated on first use.
        return instance;
    }

    ENoseColor(ENoseColor const&) = delete;
    void operator=(ENoseColor const&) = delete;

    //
    //  functionality definitions
    //
    void setFunctionalisation(const std::vector<int> &value);
    void setSensorFailures(const std::vector<bool> &value);

    QColor getSensorColor(int i);
    QColor getFuncColor(int func);
    QColor getClassColor(int i, int n);

    std::vector<int> functionalisation;
    std::vector<bool> sensorFailures;

private:
    ENoseColor() {}

    QList<QColor> smallColorList {
        QColor("#006400"),  // darkgreen
        QColor("#1e90ff"),  // dodgerblue
        QColor("#ff0000"),  // red
        QColor("#ffd700"),  // gold
        QColor("#c71585"),  // mediumvioletred
        QColor("#00ff00"),  // lime
        QColor("#0000ff"),  // blue
        QColor("#7fffd4")   // aquamarine
    };

    QList<QColor> bigColorList {
        QColor("#2f4f4f"),  // darkslategray
        QColor("#6b8e23"),  // olivedrab
        QColor("#7f0000"),  // maroon2
        QColor("#4b0082"),  // indigo
        QColor("#ff0000"),  // red
        QColor("#ffa500"),  // orange
        QColor("#c71585"),  // mediumvioletred
        QColor("#00ff00"),  // lime
        QColor("#00fa9a"),  // mediumspringgreen
        QColor("#e9967a"),  // darksalmon
        QColor("#00ffff"),  // aqua
        QColor("#0000ff"),  // blue
        QColor("#b0c4de"),  // lightsteelblue
        QColor("#ff00ff"),  // fuchsia
        QColor("#1e90ff"),  // dodgerblue
        QColor("#ffff54")   // laserlemon
    };

public:

};

#endif // SENSORCOLOR_H
