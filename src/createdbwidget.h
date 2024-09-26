#ifndef CREATEDBWIDGET_H
#define CREATEDBWIDGET_H

#include <QWidget>

namespace Ui {
class createDBwidget;
}

class createDBwidget : public QWidget
{
    Q_OBJECT

public:
    explicit createDBwidget(QWidget *parent = nullptr);
    ~createDBwidget();

private slots:
    void on_pushButton_3_clicked();

    void on_pushButton_clicked();

    void on_pushButton_4_clicked();

    void on_lineEdit_2_textChanged(const QString &arg1);

    void on_lineEdit_3_textChanged(const QString &arg1);

    void on_pushButton_Unlock_clicked();

    void on_cancel_button_clicked();

signals:
    void switchToWelcomeScreenRequest();
    void toPasswordGenerator();
    void newDatabase(QString file_path, QString passphrase, QString name);

private:
    Ui::createDBwidget *ui;
    bool pass_mode = true;
    QPixmap pass_mode_view;
    QPixmap pass_mode_hide;
    void ChangeButtonPassMode();
    void ChangeButtonPassGen();
    void ChangeButtonFile();
    void clearLineEdits();
};

#endif // CREATEDBWIDGET_H
