#ifndef UPLOADDBDIALOG_H
#define UPLOADDBDIALOG_H

#include <QWidget>

namespace Ui {
class UploadDbDialog;
}

class UploadDbDialog : public QWidget
{
    Q_OBJECT

public:
    explicit UploadDbDialog(QWidget *parent = nullptr);
    ~UploadDbDialog();

public slots:

signals:
    void confirmUpload(QString method);

private slots:
    void on_cancelButton_clicked();

    void on_okButton_clicked();

private:
    Ui::UploadDbDialog *ui;
};

#endif // UPLOADDBDIALOG_H
