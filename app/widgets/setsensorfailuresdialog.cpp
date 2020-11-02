#include "setsensorfailuresdialog.h"

#include <QtCore>
#include <QtGui>
#include <QDialog>
#include <QGridLayout>
#include <QPushButton>

#include "../classes/measurementdata.h"

SetSensorFailuresDialog::SetSensorFailuresDialog(const std::vector<bool> &failures, QWidget *parent) :
    QDialog(parent),
    nChannels(failures.size()),
    checkBoxes(nChannels, nullptr)
{
    // create checkBoxes & setup gridLayout
    QGridLayout* gridLayout = new QGridLayout();
    for (uint i=0; i<nChannels; i++)
    {
        checkBoxes[i] = new QCheckBox("ch" + QString::number(i+1), this);
        gridLayout->addWidget(checkBoxes[i], i % 16, 2* i / 16);
    }
    // set stretch of empty columns
    for (uint i=0; i<=nChannels/16; i++)
        gridLayout->setColumnStretch(2*i+1, 1);

    // set failure states
    for (uint i=0; i<nChannels; i++)
    {
        if (failures[i])
        {
            checkBoxes[i]->setCheckState(Qt::CheckState::Checked);
        } else
        {
            checkBoxes[i]->setCheckState(Qt::CheckState::Unchecked);
        }
    }

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel
                                     | QDialogButtonBox::Reset, this);
    resetButton = buttonBox->button(QDialogButtonBox::Reset);

    QVBoxLayout* pageLayout = new QVBoxLayout(this);
    pageLayout->addLayout(gridLayout);
    pageLayout->addWidget(buttonBox);

    setLayout(pageLayout);

    makeConnections();
}

SetSensorFailuresDialog::~SetSensorFailuresDialog()
{
}

std::vector<bool> SetSensorFailuresDialog::getSensorFailures()
{
    std::vector<bool> sensorFailures;

    for (uint i=0; i<checkBoxes.size(); i++)
        sensorFailures.push_back(checkBoxes[i]->checkState() == Qt::CheckState::Checked);

    return sensorFailures;
}

void SetSensorFailuresDialog::resetCheckboxes()
{
    for (uint i=0; i<nChannels; i++)
        checkBoxes[i]->setCheckState(Qt::CheckState::Unchecked);
}

void SetSensorFailuresDialog::makeConnections()
{
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SetSensorFailuresDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SetSensorFailuresDialog::reject);
    connect(resetButton, &QPushButton::clicked, this, &SetSensorFailuresDialog::resetCheckboxes);
}
