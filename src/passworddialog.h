#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QWidget>

namespace Ui {
class PasswordDialog;
}

class PasswordDialog : public QWidget
{
    Q_OBJECT

public:
    explicit PasswordDialog(QWidget *parent = nullptr);
    ~PasswordDialog();

private:
    Ui::PasswordDialog *ui;
};

#endif // PASSWORDDIALOG_H
