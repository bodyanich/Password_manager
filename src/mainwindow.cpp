#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "winDark.h"
#include <QDebug>
#include <QMessageBox>
#include <QModelIndex>

#include "databasewidget.h"
#include <QAction>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    if(winDark::isDarkTheme())
            winDark::setDark_Titlebar(reinterpret_cast<HWND>(winId()));
    ui->setupUi(this);
    ui->menubar->setStyleSheet("QMenuBar::item:selected{background: rgb(0, 85, 0);}"
                               "QMenuBar{background-color: rgb(46, 47, 48); "
                               "color: rgb(216, 216, 216);}" "QMenu::item:selected{background: rgb(0, 85, 0);}");
    switchToWeclomeScreen();

    QIcon icon("C:\\Users\\Bodyanich\\Documents\\LightPass\\Icons\\locked2.ico");
    QApplication::setWindowIcon(icon);


    //welcomeWidget
    connect(ui->welcomeWidget, SIGNAL(newDatabase()), SLOT(switchToNewDatabase()));
    connect(ui->welcomeWidget, SIGNAL(openDatabaseFile(QString)), SLOT(switchToDatabaseFile(QString)));
    connect(ui->databaseWidget, SIGNAL(newDatabasePath(QString)), ui->welcomeWidget, SLOT(addNewDatabasePath(QString)));

    //unlockWidget
    connect(this, SIGNAL(needToUnlock(QString)), ui->unlockWidget, SLOT(prepareToUnlock(QString)));
    connect(ui->unlockWidget, SIGNAL(switchToWelcomeScreenRequest()), this, SLOT(switchToWeclomeScreen()));
    connect(ui->unlockWidget, SIGNAL(switchToDatabaseScreenRequest()), this, SLOT(switchToDatabaseScreen()));
    connect(ui->unlockWidget, SIGNAL(unlockRequest(QString,QString)), ui->databaseWidget, SLOT(tryToUnlock(QString,QString)));

    //createWidget
    connect(ui->createWidget, SIGNAL(switchToWelcomeScreenRequest()), this, SLOT(switchToWeclomeScreen()));
    connect(ui->createWidget, SIGNAL(newDatabase(QString,QString,QString)), ui->databaseWidget, SLOT(createNewDatabase(QString,QString,QString)));
    connect(ui->createWidget, SIGNAL(toPasswordGenerator()), this, SLOT(switchToPasswordGeneratorScreen()));

    //databaseWidget
    connect(ui->databaseWidget, SIGNAL(getPasswordToDatabase(QString)), this, SLOT(switchToDatabaseFile(QString)));
    connect(ui->unlockWidget, SIGNAL(passwordIsReady(QString)), ui->databaseWidget, SLOT(ContinueSynchronize(QString)));
    connect(ui->unlockWidget, SIGNAL(passwordIsReady(QString)), this, SLOT(switchToDatabaseScreen()));

    connect(ui->databaseWidget, SIGNAL(switchToDatabaseScreenRequest()), this, SLOT(switchToDatabaseScreen()));
    connect(ui->actionNew_Entry, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::needDataToAddEntry);
    connect(ui->actionEdit_Entry, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::needDataToUpdateEntry);
    connect(ui->actionDelete_Entry, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::removeEntry);

    connect(ui->actionNew_Group, &QAction::triggered, this, &MainWindow::switchToEditGroupScreen);
    connect(ui->actionEdit_Group, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::needDataToUpdateGroup);
    connect(ui->actionDelete_Group, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::removeGroup);

    connect(ui->actionNew_Database, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::closeDbConnection);
    connect(ui->actionNew_Database, &QAction::triggered, this, &MainWindow::switchToNewDatabase);
    connect(ui->actionOpen_Database, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::openFile);
    connect(ui->actionSave_Database, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::saveDB);
    connect(ui->actionLock_Database, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::lockDB);
    connect(ui->databaseWidget, SIGNAL(ToUnlockScreen(QString)), this, SLOT(switchToDatabaseFile(QString)));
    connect(ui->actionClose_Database, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::closeDbConnection);
    connect(ui->actionClose_Database, &QAction::triggered, this, &MainWindow::switchToWeclomeScreen);
    connect(ui->actionQuit, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(ui->actionSave_Database_As, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::saveDBas);
    connect(ui->actionPassword_Generetor, &QAction::triggered, this, &MainWindow::switchToPasswordGeneratorScreen);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::aboutMasssge);

    connect(ui->actionUpload_To_Dropbox, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::requestToUploadToDropbox);
    connect(ui->actionDownload_From_Dropbox, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::requestToDownloadFromDropbox);
    connect(ui->actionSync_With_Dropbox, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::requestToSyncWithDropbox);

    connect(ui->databaseWidget, SIGNAL(addEntryToDatabase(bool,int, QString)), ui->editEntryWidget, SLOT(setAddData(bool,int, QString)));
    connect(ui->databaseWidget, SIGNAL(addEntryToDatabase(bool,int, QString)), this, SLOT(switchToEditScreen()));

    connect(ui->databaseWidget, SIGNAL(updateEntryInDatabase(bool,int,QSqlRecord,QString)), ui->editEntryWidget, SLOT(setUpdateData(bool,int,QSqlRecord,QString)));
    connect(ui->databaseWidget, SIGNAL(updateEntryInDatabase(bool,int,QSqlRecord,QString)), this, SLOT(switchToEditScreen()));

    connect(ui->databaseWidget, SIGNAL(updateGroupInDatabase(bool,int,QSqlRecord,QString)), ui->editGroupWidget, SLOT(setUpdateData(bool,int,QSqlRecord,QString)));
    connect(ui->databaseWidget, SIGNAL(updateGroupInDatabase(bool,int,QSqlRecord,QString)), this, SLOT(switchToEditGroupScreen()));

    connect(ui->databaseWidget, SIGNAL(openDatabaseFile(QString)), SLOT(switchToDatabaseFile(QString)));
    connect(ui->databaseWidget, SIGNAL(AddPath(QString)), ui->welcomeWidget, SLOT(checkIfExistsAndAddPath(QString)));

    connect(ui->databaseWidget, SIGNAL(tableContextMenu(QPoint, QModelIndex)), SLOT(showTableContextMenu(QPoint, QModelIndex)));


    //EditEntryWidget
    connect(ui->editEntryWidget, SIGNAL(dataForAddingIsReady(QSqlRecord)), ui->databaseWidget, SLOT(addEntry(QSqlRecord)));
    connect(ui->editEntryWidget, SIGNAL(dataForUpdatingIsReady(QSqlRecord, int)), ui->databaseWidget, SLOT(UpdateEntry(QSqlRecord, int)));
    connect(ui->editEntryWidget, SIGNAL(switchToDatabaseScreenRequest()), this, SLOT(switchToDatabaseScreen()));
    connect(ui->editEntryWidget, SIGNAL(toPasswordGenerator()), this, SLOT(switchToPasswordGeneratorScreen()));

    //EditGroupWidget
    connect(ui->editGroupWidget, SIGNAL(dataForAddingIsReady(QSqlRecord)), ui->databaseWidget, SLOT(addGroup(QSqlRecord)));
    connect(ui->editGroupWidget, SIGNAL(dataForUpdatingIsReady(QSqlRecord, int)), ui->databaseWidget, SLOT(UpdateGroup(QSqlRecord, int)));
    connect(ui->editGroupWidget, SIGNAL(switchToDatabaseScreenRequest()), this, SLOT(switchToDatabaseScreen()));

    //CopyActions
    connect(ui->actionCopy_Username, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::copyUsername);
    connect(ui->actionCopy_Password, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::copyPassword);
    connect(ui->actionCopy_Url, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::copyUrl);
    connect(ui->actionCopy_Notes, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::copyNotes);
    connect(ui->actionCopy_Title, &QAction::triggered, ui->databaseWidget, &DatabaseWidget::copyTitle);

    //passwordGenerator
    connect(ui->widget, SIGNAL(closed()), this, SLOT(switchToPreviousScreen()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::switchToNewDatabase()
{
    //m_ui->tabWidget->newDatabase();
    //switchToDatabases();
    disableActions(true);
    ui->stackedWidget->setCurrentIndex(CreateScreen);
}

void MainWindow::switchToDatabaseFile(const QString file)
{
    //m_ui->tabWidget->addDatabaseTab(file);
    //switchToDatabases();
    disableActions(true);
    emit needToUnlock(file);
    ui->stackedWidget->setCurrentIndex(UnlockScreen);
}

void MainWindow::switchToWeclomeScreen()
{
    disableActions(true);
    ui->stackedWidget->setCurrentIndex(WelcomeScreen);
}

void MainWindow::switchToDatabaseScreen()
{
    disableActions(false);
    ui->stackedWidget->setCurrentIndex(DatabaseScreen);
}

void MainWindow::switchToEditScreen()
{
    ui->stackedWidget->setCurrentIndex(EditEntryScreen);
}

void MainWindow::switchToEditGroupScreen()
{
    ui->stackedWidget->setCurrentIndex(EditGroupScreen);
}

void MainWindow::switchToPasswordGeneratorScreen()
{
    previous_screen_index = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(PasswordGeneratorScreen);
}

void MainWindow::switchToPreviousScreen()
{
    ui->stackedWidget->setCurrentIndex(previous_screen_index);
}

void MainWindow::disableActions(bool mode)
{
    ui->actionNew_Database->setDisabled(mode);
    ui->actionOpen_Database->setDisabled(mode);
    ui->actionSave_Database->setDisabled(mode);
    ui->actionSave_Database_As->setDisabled(mode);
    ui->actionLock_Database->setDisabled(mode);
    ui->actionClose_Database->setDisabled(mode);
    ui->actionUpload_To_Dropbox->setDisabled(mode);
    //ui->actionDownload_From_Dropbox->setDisabled(mode);
    ui->actionSync_With_Dropbox->setDisabled(mode);

    ui->actionNew_Entry->setDisabled(mode);
    ui->actionEdit_Entry->setDisabled(mode);
    ui->actionDelete_Entry->setDisabled(mode);
    ui->actionCopy_Title->setDisabled(mode);
    ui->actionCopy_Username->setDisabled(mode);
    ui->actionCopy_Password->setDisabled(mode);
    ui->actionCopy_Url->setDisabled(mode);
    ui->actionCopy_Notes->setDisabled(mode);

    ui->actionNew_Group->setDisabled(mode);
    ui->actionEdit_Group->setDisabled(mode);
    ui->actionDelete_Group->setDisabled(mode);

    ui->actionSettings->setDisabled(mode);
}

void MainWindow::aboutMasssge()
{
    QMessageBox::aboutQt(nullptr);
}

void MainWindow::showTableContextMenu(const QPoint globalPos, const QModelIndex index)
{
    if (index.isValid()) {
        QMenu contextMenu(tr("Context menu"), this);
        contextMenu.addAction(ui->actionCopy_Username);
        contextMenu.exec(globalPos);
    }
}


void MainWindow::on_actionSave_Database_As_triggered()
{

}

