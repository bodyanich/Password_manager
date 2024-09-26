#include "editgroup.h"
#include "ui_editgroup.h"
#include <QSqlRecord>
#include <QSqlField>
#include <QDateTime>

EditGroup::EditGroup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditGroup)
{
    ui->setupUi(this);
    mode = true;
}

EditGroup::~EditGroup()
{
    delete ui;
}

void EditGroup::on_apply_button_clicked()
{
    QSqlRecord record;

    if(mode) // 0 - update, 1 - add
    {
        record = getRecordFromLines();
        record.field("name").setGenerated(false);
        record.field("notes").setGenerated(false);

        QSqlField f7("created", QVariant::DateTime);
        QSqlField f8("modified", QVariant::DateTime);
        f7.setValue(QVariant(QDateTime::currentDateTime()));
        f8.setValue(QVariant(QDateTime::currentDateTime()));
        record.append(f7);
        record.append(f8);
        record.field("created").setGenerated(false);
        record.field("modified").setGenerated(false);

        emit dataForAddingIsReady(record);
    }
    else
    {
        record = getRecordFromLines();
        QSqlField f7("created", QVariant::DateTime);
        QSqlField f8("modified", QVariant::DateTime);
        f7.setValue(ui->lineEdit_created->text());
        f8.setValue(QVariant(QDateTime::currentDateTime()));
        record.append(f7);
        record.append(f8);

        emit dataForUpdatingIsReady(record, editedGroupRow);
        ui->apply_button->setText("Add");
        ui->header_label->setText("Add group");
    }
    mode = true;
    clearAllLineEdits();
}


void EditGroup::on_cancel_button_clicked()
{
    clearAllLineEdits();
    if(!mode)
    {
        mode = true;
        ui->apply_button->setText("Add");
        ui->header_label->setText("Add group");
    }
    emit switchToDatabaseScreenRequest();
}

void EditGroup::setUpdateData(bool m, int row, QSqlRecord record, QString name)
{
    ui->apply_button->setText("Apply");
    ui->header_label->setText(name + " - Edit group");
    mode = m;
    editedGroupRow = row;

    ui->lineEdit_name->setText(record.value(1).toString());
    ui->lineEdit_notes->setText(record.value(2).toString());
    ui->lineEdit_created->setText(record.value(3).toString());
    ui->lineEdit_modified->setText(record.value(4).toString());
}

void EditGroup::clearAllLineEdits()
{
    ui->lineEdit_name->clear();
    ui->lineEdit_notes->clear();
    ui->lineEdit_created->clear();
    ui->lineEdit_modified->clear();
}

QSqlRecord EditGroup::getRecordFromLines()
{
    QSqlRecord record;
    //QSqlField f0("id", QVariant::Int);
    QSqlField f1("name", QVariant::String);
    QSqlField f2("notes", QVariant::String);

    //f0.setValue(QVariant(5));
    f1.setValue(QVariant(ui->lineEdit_name->text()));
    f2.setValue(QVariant(ui->lineEdit_notes->text()));

    //record.append(f0);
    record.append(f1);
    record.append(f2);

    return record;
}

