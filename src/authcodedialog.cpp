#include "authcodedialog.h"
#include "ui_authcodedialog.h"
#include "winDark.h"

AuthCodeDialog::AuthCodeDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AuthCodeDialog)
{
    ui->setupUi(this);
    if(winDark::isDarkTheme())
        winDark::setDark_Titlebar(reinterpret_cast<HWND>(winId()));
}

AuthCodeDialog::~AuthCodeDialog()
{
    delete ui;
}

void AuthCodeDialog::on_cancelButton_clicked()
{
    this->close();
}


void AuthCodeDialog::on_okButton_clicked()
{
    if(ui->lineEdit->text().isEmpty())
    {
        ui->error_label->setText("No Code Provided!");
    }
    else
    {
        emit authCodeIsReady(ui->lineEdit->text());
        ui->lineEdit->clear();
        this->close();
    }
}

