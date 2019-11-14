#include "editannotationdatawindow.h"
#include "ui_editannotationdatawindow.h"
#include "classeditor.h"
#include "attributeeditor.h"

editAnnotationDataWindow::editAnnotationDataWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::editAnnotationDataWindow)
{
    ui->setupUi(this);

    ui->actionSave->setEnabled(false);
}

editAnnotationDataWindow::~editAnnotationDataWindow()
{    void on_listWidget_itemChanged(QListWidgetItem *item);

    delete ui;
}

void editAnnotationDataWindow::setModel(QAbstractItemModel* model)
{
    ui->tableView->setModel(model);
    aData = static_cast<AnnotationDatasetModel*>(model);
}

void editAnnotationDataWindow::on_actionLoad_triggered()
{
    aData->loadData(this);
}

void editAnnotationDataWindow::on_actionSave_triggered()
{
    aData->saveData(this);
}

void editAnnotationDataWindow::on_actionEditAttributes_triggered()
{
    AttributeEditor editor;

    editor.setAttributes(aData->getExtraAttributes());

    connect(&editor, &AttributeEditor::renameAttribute, aData, &AnnotationDatasetModel::renameClass);
    connect(&editor, &AttributeEditor::deleteAttribute, aData, &AnnotationDatasetModel::deleteClass);
    connect(&editor, &AttributeEditor::addAttribute, aData, &AnnotationDatasetModel::addClass);

    editor.exec();

}

void editAnnotationDataWindow::on_actionEditClasses_triggered()
{
    ClassEditor editor;

    editor.setClassList(aData->getClasses());

    connect(&editor, &ClassEditor::renameClass, aData, &AnnotationDatasetModel::renameClass);
    connect(&editor, &ClassEditor::deleteClass, aData, &AnnotationDatasetModel::deleteClass);
    connect(&editor, &ClassEditor::addClass, aData, &AnnotationDatasetModel::addClass);

    editor.exec();

    disconnect(&editor, &ClassEditor::renameClass, aData, &AnnotationDatasetModel::renameClass);
    disconnect(&editor, &ClassEditor::deleteClass, aData, &AnnotationDatasetModel::deleteClass);
    disconnect(&editor, &ClassEditor::addClass, aData, &AnnotationDatasetModel::addClass);
}
