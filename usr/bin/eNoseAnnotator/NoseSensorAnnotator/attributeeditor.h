#ifndef ATTRIBUTEEDITOR_H
#define ATTRIBUTEEDITOR_H

#include <QtCore>
#include <QDialog>
#include <QtCore>

namespace Ui {
class AttributeEditor;
}

class AttributeEditor : public QDialog
{
    Q_OBJECT

public:
    explicit AttributeEditor(QWidget *parent = nullptr);
    ~AttributeEditor();

    void setAttributes (QMap<QString, QString>);

signals:
    void renameAttribute (QString oldName, QString newName);
    void changeDefaultValue (QString attribute, QString defaultValue);
    void deleteAttribute (QString attributeName);
    void addAttribute (QString attributeName, QString defaultValue);

private slots:

    void on_addButton_clicked();

    void on_deleteButton_clicked();

    void on_closeButton_clicked();

    void on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_tableWidget_cellChanged(int row, int column);

private:
    Ui::AttributeEditor *ui;

    QMap<QString, QString> attributes;
    QString oldValue;   // stores the old value of currently selected cell
};

#endif // ATTRIBUTEEDITOR_H
