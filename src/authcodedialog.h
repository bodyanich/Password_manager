#ifndef AUTHCODEDIALOG_H
#define AUTHCODEDIALOG_H

#include <QDialog>

namespace Ui {
class AuthCodeDialog;
}

class AuthCodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AuthCodeDialog(QWidget *parent = nullptr);
    ~AuthCodeDialog();

signals:
    void authCodeIsReady(QString code);

private slots:
    void on_cancelButton_clicked();

    void on_okButton_clicked();

private:
    Ui::AuthCodeDialog *ui;
};

#endif // AUTHCODEDIALOG_H
