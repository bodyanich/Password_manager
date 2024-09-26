#include "unlockwidget.h"
#include "ui_unlockwidget.h"
#include <QMessageBox>

UnlockWidget::UnlockWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UnlockWidget)
{
    ui->setupUi(this);
    pass_mode_view = QPixmap(":/Icons/view_3.png");
    pass_mode_hide = QPixmap(":/Icons/hide.png");
    ChangeButtonPassMode();
    file_path = "";
}

UnlockWidget::~UnlockWidget()
{
    delete ui;
}

void UnlockWidget::on_pushButton_clicked()
{
    ChangeButtonPassMode();
}

void UnlockWidget::ChangeButtonPassMode()
{
    if (pass_mode)
    {
        QIcon ButtonIcon(pass_mode_hide);
        ui->pushButton->setIcon(ButtonIcon);
        ui->pushButton->setIconSize(pass_mode_hide.rect().size());
        ui->lineEdit->setEchoMode(QLineEdit::Password);
        pass_mode = false;
    }
    else
    {
        QIcon ButtonIcon(pass_mode_view);
        ui->pushButton->setIcon(ButtonIcon);
        ui->pushButton->setIconSize(pass_mode_view.rect().size());
        ui->lineEdit->setEchoMode(QLineEdit::Normal);
        pass_mode = true;
    }
}

void UnlockWidget::prepareToUnlock(const QString file)
{
    if (file == "Please, enter your password again to synchronize database with dropbox.")
    {
        ui->labelHeadline->setText(file);
        ui->labelFilename->setText("");
        sync_mode = true;
    }
    else
    {
        file_path = file;
        ui->labelFilename->setText(file_path);
    }
}

void UnlockWidget::on_pushButton_2_clicked()
{
    ui->lineEdit->clear();
    if(sync_mode)
    {
        sync_mode = false;
        emit switchToDatabaseScreenRequest();
        ui->labelHeadline->setText("Unlock ToughPass Database");
    }
    else
    {
        emit switchToWelcomeScreenRequest();
    }
}


void UnlockWidget::on_pushButton_Unlock_clicked()
{
    if (ui->lineEdit->text() == "")
    {
        QMessageBox::critical(nullptr, "Database password is not entered", "You have to enter the database password!", QMessageBox::Ok);
    }
    else if (sync_mode)
    {
        sync_mode = false;
        emit passwordIsReady(ui->lineEdit->text());
        ui->lineEdit->clear();
        ui->labelHeadline->setText("Unlock ToughPass Database");
    }
    else
    {
        emit unlockRequest(ui->labelFilename->text(), ui->lineEdit->text());
        ui->lineEdit->clear();
    }
}

