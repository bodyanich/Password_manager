#include "editentry.h"
#include "ui_editentry.h"
#include <QSqlRecord>
#include <QSqlField>
#include <QDateTime>
#include "zxcvbn/zxcvbn.h"

EditEntry::EditEntry(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditEntry)
{
    ui->setupUi(this);

    pass_mode_view = QPixmap(":/Icons/view_3.png");
    pass_mode_hide = QPixmap(":/Icons/hide.png");
    ChangeButtonPassMode();
    ChangeButtonPassGen();

    ui->progressBar->setValue(0);
    ui->progressBar->setTextVisible(false);
    ui->progressBar->setMaximumHeight(0);
}

EditEntry::~EditEntry()
{
    delete ui;
}

void EditEntry::setAddData(bool m, int id, QString name)
{
    ui->apply_button->setText("Add");
    ui->header_label->setText(name + " - Add record");
    mode = m;
    group_id = id;
    group_name = name;
}

void EditEntry::setUpdateData(bool m, int row, QSqlRecord record, QString name)
{
    ui->apply_button->setText("Apply");
    ui->header_label->setText(name + " - " + record.value(2).toString() + " - Edit record");
    mode = m;
    group_name = name;
    editedEntryRow = row;

    ui->lineEdit_title->setText(record.value(2).toString());
    ui->lineEdit_username->setText(record.value(3).toString());
    ui->lineEdit_password->setText(record.value(4).toString());
    ui->lineEdit_url->setText(record.value(5).toString());
    ui->textEdit->setText(record.value(6).toString());
    ui->lineEdit_created->setText(record.value(7).toString());
    ui->lineEdit_modified->setText(record.value(8).toString());
}

void EditEntry::clearAllLineEdits()
{
    ui->lineEdit_title->clear();
    ui->lineEdit_username->clear();
    ui->lineEdit_password->clear();
    ui->lineEdit_url->clear();
    ui->textEdit->clear();
    ui->lineEdit_created->clear();
    ui->lineEdit_modified->clear();
}

void EditEntry::on_apply_button_clicked()
{
    QSqlRecord record;

    if(mode) // 0 - update, 1 - add
    {
        record = getRecordFromLines();
        record.field("group_id").setGenerated(false);
        record.field("title").setGenerated(false);
        record.field("username").setGenerated(false);
        record.field("password").setGenerated(false);
        record.field("url").setGenerated(false);
        record.field("notes").setGenerated(false);

        QSqlField f7("created", QVariant::DateTime);
        QSqlField f8("modified", QVariant::DateTime);
        QSqlField f9("entropy", QVariant::Double);
        f7.setValue(QVariant(QDateTime::currentDateTime()));
        f8.setValue(QVariant(QDateTime::currentDateTime()));
        f9.setValue(QVariant(ZxcvbnMatch(ui->lineEdit_password->text().toUtf8(), nullptr, nullptr)));
        record.append(f7);
        record.append(f8);
        record.append(f9);
        record.field("created").setGenerated(false);
        record.field("modified").setGenerated(false);
        record.field("entropy").setGenerated(false);

        emit dataForAddingIsReady(record);
    }
    else
    {
        record = getRecordFromLines();
        QSqlField f7("created", QVariant::DateTime);
        QSqlField f8("modified", QVariant::DateTime);
        QSqlField f9("entropy", QVariant::Double);
        f7.setValue(ui->lineEdit_created->text());
        f8.setValue(QVariant(QDateTime::currentDateTime()));
        f9.setValue(QVariant(ZxcvbnMatch(ui->lineEdit_password->text().toUtf8(), nullptr, nullptr)));
        record.append(f7);
        record.append(f8);
        record.append(f9);

        emit dataForUpdatingIsReady(record, editedEntryRow);
    }
    clearAllLineEdits();
}


void EditEntry::on_cancel_button_clicked()
{
    clearAllLineEdits();
    emit switchToDatabaseScreenRequest();
}

QSqlRecord EditEntry::getRecordFromLines()
{
    QSqlRecord record;
    //QSqlField f0("id", QVariant::Int);
    QSqlField f1("group_id", QVariant::Int);
    QSqlField f2("title", QVariant::String);
    QSqlField f3("username", QVariant::String);
    QSqlField f4("password", QVariant::String);
    QSqlField f5("url", QVariant::String);
    QSqlField f6("notes", QVariant::String);

    //f0.setValue(QVariant(5));
    f1.setValue(QVariant(group_id));
    f2.setValue(QVariant(ui->lineEdit_title->text()));
    f3.setValue(QVariant(ui->lineEdit_username->text()));
    f4.setValue(QVariant(ui->lineEdit_password->text()));
    f5.setValue(QVariant(ui->lineEdit_url->text()));
    f6.setValue(QVariant(ui->textEdit->toPlainText()));

    //record.append(f0);
    record.append(f1);
    record.append(f2);
    record.append(f3);
    record.append(f4);
    record.append(f5);
    record.append(f6);

    return record;
}


void EditEntry::on_lineEdit_password_textChanged(const QString &arg1)
{
    if(arg1.isEmpty())
    {
        ui->progressBar->setValue(0);
        ui->progressBar->setMaximumHeight(0);
    }
    else
    {
        auto entropy = ZxcvbnMatch(arg1.toUtf8(), nullptr, nullptr);
        ui->progressBar->setValue(std::min(int(entropy), ui->progressBar->maximum()));
        ui->progressBar->setMaximumHeight(5);
        if (entropy < 25)
        {
            ui->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #C43F31}");
        } else if (entropy < 50)
        {
            ui->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #DB9837}");
        } else if (entropy < 80)
        {
            ui->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #F0C400}");
        } else if (entropy < 100)
        {
            ui->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #608A22}");
        }
        else
        {
            ui->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #1F8023}");
        }
    }
}

void EditEntry::calculateEtropy(QString pwd)
{

}

void EditEntry::ChangeButtonPassMode()
{
    if (pass_mode)
    {
        QIcon ButtonIcon(pass_mode_hide);
        ui->pass_button->setIcon(ButtonIcon);
        ui->pass_button->setIconSize(pass_mode_hide.rect().size());
        ui->lineEdit_password->setEchoMode(QLineEdit::Password);
        pass_mode = false;
    }
    else
    {
        QIcon ButtonIcon(pass_mode_view);
        ui->pass_button->setIcon(ButtonIcon);
        ui->pass_button->setIconSize(pass_mode_view.rect().size());
        ui->lineEdit_password->setEchoMode(QLineEdit::Normal);
        pass_mode = true;
    }
}

void EditEntry::ChangeButtonPassGen()
{
    ui->gen_button->setIcon(QIcon(":/Icons/die_1.png"));
    ui->gen_button->setIconSize(QSize(24, 24));
}


void EditEntry::on_pass_button_clicked()
{
    ChangeButtonPassMode();
}


void EditEntry::on_gen_button_clicked()
{
    emit toPasswordGenerator();
}

