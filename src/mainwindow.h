#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum StackedWidgetIndex
        {
            WelcomeScreen = 0,
            DatabaseScreen = 1,
            UnlockScreen = 2,
            CreateScreen = 3,
            EditEntryScreen = 4,
            EditGroupScreen = 5,
            PasswordGeneratorScreen = 6,
            SettingsScreen = 7
        };

public slots:
    void disableActions(bool);

private slots:
    void switchToNewDatabase();
    void switchToDatabaseFile(const QString file);
    void switchToWeclomeScreen();
    void switchToDatabaseScreen();
    void switchToEditScreen();
    void switchToEditGroupScreen();
    void switchToPasswordGeneratorScreen();
    void switchToPreviousScreen();
    void aboutMasssge();
    void showTableContextMenu(const QPoint globalPos, const QModelIndex index);

    void on_actionSave_Database_As_triggered();

signals:
    void needToUnlock(const QString& file);

private:
    int previous_screen_index;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
