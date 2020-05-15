#include "setsensorfailuresdialog.h"

#include <QtCore>
#include <QtGui>
#include <QDialog>
#include <QGridLayout>
#include <QPushButton>

#include "../classes/measurementdata.h"

SetSensorFailuresDialog::SetSensorFailuresDialog(QWidget *parent, ulong nChannels, QString failureString) :
    QDialog(parent),
    nChannels(nChannels),
    checkBoxes(nChannels, nullptr)
{
    auto failures = MeasurementData::sensorFailureArray(failureString);

    // create checkBoxes & setup gridLayout
    QGridLayout* gridLayout = new QGridLayout();
    for (uint i=0; i<nChannels; i++)
    {
        checkBoxes[i] = new QCheckBox("ch" + QString::number(i));
        gridLayout->addWidget(checkBoxes[i], i % 16, 2* i / 16);
    }
    // set stretch of empty columns
    for (uint i=0; i<=nChannels/16; i++)
        gridLayout->setColumnStretch(2*i+1, 1);

    // set failure states
    for (int i=0; i<nChannels; i++)
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
                                     | QDialogButtonBox::Reset);
    resetButton = buttonBox->button(QDialogButtonBox::Reset);

    QVBoxLayout* pageLayout = new QVBoxLayout();
    pageLayout->addLayout(gridLayout);
    pageLayout->addWidget(buttonBox);

    setLayout(pageLayout);

    makeConnections();
}

SetSensorFailuresDialog::~SetSensorFailuresDialog()
{
    for (QCheckBox* checkBox : checkBoxes)
        checkBox->deleteLater();

    buttonBox->deleteLater();
}

std::vector<bool> SetSensorFailuresDialog::getSensorFailures()
{
    std::vector<bool> sensorFailures;

    for (int i=0; i<checkBoxes.size(); i++)
        sensorFailures.push_back(checkBoxes[i]->checkState() == Qt::CheckState::Checked);

    return sensorFailures;
}

void SetSensorFailuresDialog::resetCheckboxes()
{
    for (int i=0; i<MVector::nChannels; i++)
        checkBoxes[i]->setCheckState(Qt::CheckState::Unchecked);
}

void SetSensorFailuresDialog::makeConnections()
{
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SetSensorFailuresDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SetSensorFailuresDialog::reject);
    connect(resetButton, &QPushButton::released, this, &SetSensorFailuresDialog::resetCheckboxes);
}
