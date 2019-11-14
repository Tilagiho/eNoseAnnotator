#ifndef CLASSEDITOR_H
#define CLASSEDITOR_H

#include <QDialog>
#include <QListWidget>

namespace Ui {
class ClassEditor;
}

class ClassEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ClassEditor(QWidget *parent = nullptr);
    ~ClassEditor();

    QStringList getClassList() const;
    void setClassList(const QStringList &value);

signals:
    void renameClass (QString oldName, QString newName);
    void deleteClass (QString className);
    void addClass (QString className);

private slots:
    void on_addButton_clicked();

    void on_deleteButton_clicked();

    void on_renameButton_clicked();

    void on_listWidget_currentRowChanged(int currentRow);

    void on_closeButton_clicked();

private:
    Ui::ClassEditor *ui;
    QStringList classList;
};

#endif // CLASSEDITOR_H
