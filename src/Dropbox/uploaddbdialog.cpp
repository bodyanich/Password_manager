#include "uploaddbdialog.h"
#include "ui_uploaddbdialog.h"

UploadDbDialog::UploadDbDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UploadDbDialog)
{
    ui->setupUi(this);
}

UploadDbDialog::~UploadDbDialog()
{
    delete ui;
}

void UploadDbDialog::on_cancelButton_clicked()
{
    this->close();
}


void UploadDbDialog::on_okButton_clicked()
{
    if(ui->radioButton->isChecked())
    {
        emit confirmUpload("add");
    }
    else if (ui->radioButton_2->isChecked())
    {
        emit confirmUpload("overwrite");
    }
    this->close();
}

