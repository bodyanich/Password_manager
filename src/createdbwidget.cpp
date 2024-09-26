#include "createdbwidget.h"
#include "ui_createdbwidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "zxcvbn/zxcvbn.h"

createDBwidget::createDBwidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::createDBwidget)
{
    ui->setupUi(this);
    pass_mode_view = QPixmap(":/Icons/view_3.png");
    pass_mode_hide = QPixmap(":/Icons/hide.png");
    ChangeButtonPassMode();
    ChangeButtonPassGen();
    ChangeButtonFile();
    ui->lineEdit_2->setEchoMode(QLineEdit::Password);
    ui->lineEdit_3->setEchoMode(QLineEdit::Password);
    ui->progressBar->setValue(0);
    ui->progressBar->setTextVisible(false);
    ui->progressBar->setMaximumHeight(0);
}

createDBwidget::~createDBwidget()
{
    delete ui;
}

void createDBwidget::on_pushButton_3_clicked()
{
    ChangeButtonPassMode();
}

void createDBwidget::on_pushButton_clicked()
{
    emit toPasswordGenerator();
}


void createDBwidget::on_pushButton_4_clicked()
{
    QString file_str = QFileDialog::getSaveFileName(0, "Save database", ui->lineEdit->text() + ".db");
    ui->lineEdit_4->setText(file_str);
}

void createDBwidget::ChangeButtonPassMode()
{
    if (pass_mode)
    {
        QIcon ButtonIcon(pass_mode_hide);
        ui->pushButton_3->setIcon(ButtonIcon);
        ui->pushButton_3->setIconSize(pass_mode_hide.rect().size());
        ui->lineEdit_2->setEchoMode(QLineEdit::Password);
        ui->lineEdit_3->setEchoMode(QLineEdit::Password);
        pass_mode = false;
    }
    else
    {
        QIcon ButtonIcon(pass_mode_view);
        ui->pushButton_3->setIcon(ButtonIcon);
        ui->pushButton_3->setIconSize(pass_mode_view.rect().size());
        ui->lineEdit_2->setEchoMode(QLineEdit::Normal);
        ui->lineEdit_3->setEchoMode(QLineEdit::Normal);
        pass_mode = true;
    }
}

void createDBwidget::ChangeButtonPassGen()
{
    ui->pushButton->setIcon(QIcon(":/Icons/die_1.png"));
    ui->pushButton->setIconSize(QSize(24, 24));
}

void createDBwidget::ChangeButtonFile()
{
    ui->pushButton_4->setIcon(QIcon(":/Icons/folder.png"));
    ui->pushButton_4->setIconSize(QSize(24, 24));
}


void createDBwidget::on_lineEdit_2_textChanged(const QString &arg1)
{
    if(arg1 != ui->lineEdit_3->text())
        ui->lineEdit_3->setStyleSheet("QLineEdit { "
                                      "background-color: rgb(125, 32, 52); "
                                      "border-width: 2px; border-style: solid;"
                                      " border-color: rgb(125, 32, 52); color: rgb(194, 194, 194); "
                                      "}"
                                      "QLineEdit:focus { "
                                      "border-color: rgb(0, 85, 0); }");
    else
        ui->lineEdit_3->setStyleSheet("QLineEdit { "
                                      "background-color: rgb(39, 39, 42); "
                                      "border-width: 2px; border-style: solid;"
                                      " border-color:  rgb(93, 93, 93); color: rgb(194, 194, 194); "
                                      "}"
                                      "QLineEdit:focus { "
                                      "border-color: rgb(0, 85, 0); }");
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
        qDebug() << "entropy = " << entropy;
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


void createDBwidget::on_lineEdit_3_textChanged(const QString &arg1)
{
    if(arg1 != ui->lineEdit_2->text())
        ui->lineEdit_3->setStyleSheet("QLineEdit { "
                                      "background-color: rgb(125, 32, 52); "
                                      "border-width: 2px; border-style: solid;"
                                      " border-color: rgb(125, 32, 52); color: rgb(194, 194, 194); "
                                      "}"
                                      "QLineEdit:focus { "
                                      "border-color: rgb(0, 85, 0); }");
    else
        ui->lineEdit_3->setStyleSheet("QLineEdit { "
                                      "background-color: rgb(39, 39, 42); "
                                      "border-width: 2px; border-style: solid;"
                                      " border-color:  rgb(93, 93, 93); color: rgb(194, 194, 194); "
                                      "}"
                                      "QLineEdit:focus { "
                                      "border-color: rgb(0, 85, 0); }");
}


void createDBwidget::on_pushButton_Unlock_clicked()
{
    if (ui->lineEdit->text() == "")
    {
        QMessageBox::critical(nullptr, "Database name is not specified", "You have to specify the database name!", QMessageBox::Ok);
    }
    else if (ui->lineEdit_2->text() == "")
    {
        QMessageBox::critical(nullptr, "Database password is not specified", "You have to specify the database password!", QMessageBox::Ok);
    }
    else if (ui->lineEdit_2->text() != ui->lineEdit_3->text())
    {
        QMessageBox::critical(nullptr, "Password mismatch", "Passwords don't match!", QMessageBox::Ok);
    }
    else if (ui->lineEdit_4->text() == "")
    {
        QMessageBox::critical(nullptr, "Database path is not specified", "You have to specify the database path!", QMessageBox::Ok);
    }
    else
    {
        emit newDatabase(ui->lineEdit_4->text(), ui->lineEdit_2->text(), ui->lineEdit->text());
        clearLineEdits();
    }
}


void createDBwidget::on_cancel_button_clicked()
{
    clearLineEdits();
    emit switchToWelcomeScreenRequest();
}

void createDBwidget::clearLineEdits()
{
    ui->lineEdit->clear();
    ui->lineEdit_2->clear();
    ui->lineEdit_3->clear();
    ui->lineEdit_4->clear();
}

