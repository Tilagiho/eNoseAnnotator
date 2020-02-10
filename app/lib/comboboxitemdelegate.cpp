#include "comboboxitemdelegate.h"
#include <QComboBox>

#include "../widgets/classselector.h"

QWidget *ComboBoxItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // create & fill ComboBox with comboBox
    QComboBox *cb = new QComboBox(parent);

    cb->addItem("");
    for (aClass c : ClassSelector::getClassList())
        cb->addItem(c.toString());

    return cb;
}


void ComboBoxItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);

    // Suche nach dem aktuellen Wert welcher im Model gespeichert ist...
    const QString currentText = index.data(Qt::EditRole).toString();
    if (currentText != "")
        cb->addItem(currentText);
    const int cbIndex = cb->findText(currentText);
    // ... und setzen der Combobox auf diesen, falls vorhanden.
    if (cbIndex >= 0)
       cb->setCurrentIndex(cbIndex);
}


void ComboBoxItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);
    model->setData(index, cb->currentText(), Qt::EditRole);
}
