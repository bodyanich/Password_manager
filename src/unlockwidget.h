#ifndef UNLOCKWIDGET_H
#define UNLOCKWIDGET_H

#include <QWidget>

namespace Ui {
class UnlockWidget;
}

class UnlockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UnlockWidget(QWidget *parent = nullptr);
    ~UnlockWidget();

private slots:
    void on_pushButton_clicked();
    void prepareToUnlock(const QString file);

    void on_pushButton_2_clicked();
    void on_pushButton_Unlock_clicked();

signals:
    void unlockRequest(const QString file, const QString passphrase);
    void passwordIsReady(QString password);
    void switchToWelcomeScreenRequest();
    void switchToDatabaseScreenRequest();

private:
    Ui::UnlockWidget *ui;
    bool pass_mode = true;
    bool sync_mode = false;
    QPixmap pass_mode_view;
    QPixmap pass_mode_hide;
    QString file_path;
    void ChangeButtonPassMode();
};

#endif // UNLOCKWIDGET_H
