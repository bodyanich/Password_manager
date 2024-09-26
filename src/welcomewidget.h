#ifndef WELCOMEWIDGET_H
#define WELCOMEWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QFile>

namespace Ui {
class WelcomeWidget;
}

class WelcomeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomeWidget(QWidget *parent = nullptr);
    ~WelcomeWidget();
    void refreshLastDatabases();

public slots:
    void on_buttonOpenDatabase_clicked();
    void checkIfExistsAndAddPath(const QString& fileName);

signals:
    void newDatabase();
    void openDatabaseFile(QString);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;


private slots:
    void openDatabaseFromFile(QListWidgetItem* item);
    void addNewDatabasePath(QString path);

private:
    Ui::WelcomeWidget *ui;
    QFile file;

    void removeFromLastDatabases(QListWidgetItem* item);
};

#endif // WELCOMEWIDGET_H
