#include "databasewidget.h"
#include "ui_databasewidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlRecord>
#include <QItemSelection>
#include <QClipboard>
#include <QFileDialog>
#include <QMenu>
#include <QMouseEvent>

DatabaseWidget::DatabaseWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DatabaseWidget)
{
    ui->setupUi(this);
    authCodeDialog = new AuthCodeDialog();
    dropbox = new DropboxApi();
    lastDropboxActions = None;

    pclipboard = QGuiApplication::clipboard();
    pass_mode_view = QPixmap(":/Icons/view_3.png");
    pass_mode_hide = QPixmap(":/Icons/hide.png");
    ChangeButtonPassMode();

    customizeTableView();
    customizeListView();

    group_name = "";
    record_title = "";

    connect(ui->tableView, &QTableView::activated, this, &DatabaseWidget::tableItemActivated);
    connect(ui->listView, &QListView::activated, this, &DatabaseWidget::listItemActivated);

    connect(authCodeDialog, SIGNAL(authCodeIsReady(QString)), dropbox, SLOT(getAccessToken(QString)));
    connect(dropbox, SIGNAL(RefreshTokenWrite(QString)), this, SLOT(updateDropboxRefToken(QString)));
    connect(dropbox, SIGNAL(OpenAuthDialog()), this, SLOT(showAuthDialog()));
    connect(dropbox, SIGNAL(SuccessfulAuth()), this, SLOT(startDropboxActionFlow()));
    connect(dropbox, SIGNAL(openDownloadedDatabase(QString)), this, SLOT(openDownloadedFile(QString)));
    connect(dropbox, SIGNAL(ReadyForSync()), this, SLOT(synchronize()));
}

DatabaseWidget::~DatabaseWidget()
{
    delete ui;
    delete authCodeDialog;
    delete dropbox;
}

void DatabaseWidget::on_pass_button_clicked()
{
    ChangeButtonPassMode();
}

void DatabaseWidget::ChangeButtonPassMode()
{
    if (pass_mode)
    {
        QIcon ButtonIcon(pass_mode_hide);
        ui->pass_button->setIcon(ButtonIcon);
        ui->pass_button->setIconSize(pass_mode_hide.rect().size());
        ui->password_lineEdit->setEchoMode(QLineEdit::Password);
        pass_mode = false;
    }
    else
    {
        QIcon ButtonIcon(pass_mode_view);
        ui->pass_button->setIcon(ButtonIcon);
        ui->pass_button->setIconSize(pass_mode_view.rect().size());
        ui->password_lineEdit->setEchoMode(QLineEdit::Normal);
        pass_mode = true;
    }
}

void DatabaseWidget::createNewDatabase(QString file_path, QString passphrase, QString name)
{
    databasePath = file_path;
    database.createConnection(file_path, passphrase);
    database.initDatabase();
    database.initModel();
    initViews();
    emit newDatabasePath(file_path);
}

void DatabaseWidget::closeDbConnection()
{
    if (database.isOpen())
    {
        disconnect(ui->listView->selectionModel(), &QItemSelectionModel::selectionChanged,
                   this, &DatabaseWidget::listViewItemChanged);
        disconnect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
                   this, &DatabaseWidget::tableViewItemChanged);
        ui->tableView->setModel(nullptr);
        ui->listView->setModel(nullptr);
        database.closeDatabase();
    }
}

void DatabaseWidget::tryToUnlock(const QString file, const QString passphrase)
{
    databasePath = file;
    if(!database.createConnection(file, passphrase))
    {
        QMessageBox::critical(nullptr, "Wrong password or filepath", "Wrong password or filepath!", QMessageBox::Ok);
    }
    else
    {
        qDebug() << "password is correct";
        database.initModel();
        initViews();
    }
}

void DatabaseWidget::initViews()
{
    database.getModel()->setHeaderData(1, Qt::Horizontal, "group");
    ui->listView->setModel(database.getModel()->relationModel(1));
    ui->listView->setModelColumn(1);
    ui->tableView->setModel(database.getModel());
    adjustHeader();

    connect(ui->listView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &DatabaseWidget::listViewItemChanged);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &DatabaseWidget::tableViewItemChanged);

    ui->listView->setCurrentIndex(database.getModel()->relationModel(1)->index(0, 0));

    if(ui->tableView->model()->rowCount() != 0)
    {
        QModelIndex index = ui->tableView->model()->index(0, 0);
        ui->tableView->setCurrentIndex(index);
    }

    emit switchToDatabaseScreenRequest();
}

void DatabaseWidget::customizeTableView()
{
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setShowGrid(false);
    ui->tableView->verticalHeader()->hide();
    ui->tableView->setAlternatingRowColors(true);

    CustomItemDelegate *delegate = new CustomItemDelegate();
    ui->tableView->setItemDelegate(delegate);
}

void DatabaseWidget::customizeListView()
{
    ui->listView->setViewMode(QListView::ListMode);
    ui->listView->setLayoutMode(QListView::SinglePass);
    ui->listView->setSpacing(5);
    ui->listView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listView->setSelectionMode(QAbstractItemView::SingleSelection);
}

void DatabaseWidget::adjustHeader()
{
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(4);
    ui->tableView->hideColumn(5);
    ui->tableView->hideColumn(9);
}

void DatabaseWidget::needDataToAddEntry()
{
    QModelIndexList index = ui->listView->selectionModel()->selectedIndexes();
    if(!index.isEmpty())
    {
        QSqlRecord record = database.getModel()->relationModel(1)->record(index[0].row());
        int id = record.value(0).toInt();
        QString name = record.value(1).toString();
        qDebug() << "Selected item record:" << id;
        emit addEntryToDatabase(true, id, name); // 0 - update, 1 - add
    }
}

void DatabaseWidget::needDataToUpdateEntry()
{
    QModelIndexList index = ui->tableView->selectionModel()->selectedIndexes();
    QModelIndexList index2 = ui->listView->selectionModel()->selectedIndexes();
    if(!index.isEmpty() && !index2.isEmpty())
    {
        int row = index[0].row();
        int row2 = index2[0].row();
        QSqlRecord record = database.getModel()->record(row);
        QSqlRecord record2 = database.getModel()->relationModel(1)->record(row2);
        QString name = record2.value(1).toString();
        emit updateEntryInDatabase(false, row, record, name); // 0 - update, 1 - add
    }
}

void DatabaseWidget::tableItemActivated(const QModelIndex &index)
{
    QModelIndexList index2 = ui->listView->selectionModel()->selectedIndexes();
    if(!index2.isEmpty())
    {
        int row = index.row();
        int row2 = index2[0].row();
        QSqlRecord record = database.getModel()->record(row);
        QSqlRecord record2 = database.getModel()->relationModel(1)->record(row2);
        QString name = record2.value(1).toString();
        emit updateEntryInDatabase(false, row, record, name); // 0 - update, 1 - add
    }
}

void DatabaseWidget::addEntry(QSqlRecord record)
{
    database.insertRecord(record);
    emit switchToDatabaseScreenRequest();
}

void DatabaseWidget::UpdateEntry(QSqlRecord record, int row)
{
    database.updateRecord(record, row);
    emit switchToDatabaseScreenRequest();
}

void DatabaseWidget::removeEntry()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows(0);

    if (!selection.empty())
    {
        QModelIndex idIndex = selection.at(0);
        QString title = idIndex.sibling(idIndex.row(), 2).data().toString();

        QMessageBox::StandardButton button;
        button = QMessageBox::question(this, tr("Delete record"),
                                       tr("Are you sure you want to "
                                          "delete '%1'?")
                                       .arg(title),
                                       QMessageBox::Yes | QMessageBox::No);

        if (button == QMessageBox::Yes)
        {
            database.removeRecord(idIndex);
        }
    } else
    {
        QMessageBox::information(this, tr("Delete record"),
                                 tr("Select the record you want to delete."));
    }
}

void DatabaseWidget::needDataToUpdateGroup()
{
    QModelIndexList index = ui->listView->selectionModel()->selectedIndexes();
    if(!index.isEmpty())
    {
        int row = index[0].row();
        QSqlRecord record = database.getModel()->relationModel(1)->record(row);
        QString name = record.value(1).toString();
        emit updateGroupInDatabase(false, row, record, name); // 0 - update, 1 - add
    }
}

void DatabaseWidget::listItemActivated(const QModelIndex &index)
{
    int row = index.row();
    QSqlRecord record = database.getModel()->relationModel(1)->record(row);
    QString name = record.value(1).toString();
    emit updateGroupInDatabase(false, row, record, name); // 0 - update, 1 - add
}

void DatabaseWidget::addGroup(QSqlRecord record)
{
    database.insertGroup(record);
    emit switchToDatabaseScreenRequest();
}

void DatabaseWidget::UpdateGroup(QSqlRecord record, int row)
{
    database.updateGroup(record, row);
    emit switchToDatabaseScreenRequest();
}

void DatabaseWidget::removeGroup()
{
    QModelIndexList selection = ui->listView->selectionModel()->selectedIndexes();

    if (!selection.empty())
    {
        QModelIndex idIndex = selection.at(0);
        QString name = idIndex.sibling(idIndex.row(), 1).data().toString();

        QMessageBox::StandardButton button;
        button = QMessageBox::question(this, tr("Delete Group"),
                                       tr("Are you sure you want to "
                                          "delete '%1'?")
                                       .arg(name),
                                       QMessageBox::Yes | QMessageBox::No);

        if (button == QMessageBox::Yes)
        {
            database.removeGroup(idIndex);
        }
    } else
    {
        QMessageBox::information(this, tr("Delete Group"),
                                 tr("Select the Group you want to delete."));
    }
}

void DatabaseWidget::listViewItemChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    QModelIndexList index = selected.indexes();
    if(!index.isEmpty())
    {
        int row = index[0].row();
        QSqlRecord record = database.getModel()->relationModel(1)->record(row);
        database.getModel()->setFilter("group_id = " + record.value(0).toString());
        database.getModel()->select();
        group_name = record.value(1).toString();
        record_title = "";
        ui->gpup_name_label->setText(group_name + " / " + record_title);
        ui->tableView->selectionModel()->clearSelection();

        ui->username_label_2->clear();
        ui->password_lineEdit->clear();
        ui->notes_label_2->clear();
        ui->url_label_2->clear();
        ui->created_label_2->clear();
        ui->modified_label_2->clear();
    }
}

void DatabaseWidget::tableViewItemChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    QModelIndexList index = selected.indexes();
    if(!index.isEmpty())
    {
        int row = index[0].row();
        QSqlRecord record = database.getModel()->record(row);
        record_title = record.value(2).toString();
        ui->gpup_name_label->setText(group_name + " / " + record_title);

        ui->username_label_2->setText(record.value(3).toString());
        ui->password_lineEdit->setText(record.value(4).toString());
        ui->notes_label_2->setText(record.value(6).toString());

        QString link = record.value(5).toString();
        ui->url_label_2->setText(QString("<a href=\"%1\">%2</a>").arg(link, link));
        ui->created_label_2->setText(record.value(7).toString());
        ui->modified_label_2->setText(record.value(8).toString());
    }
}

void DatabaseWidget::saveDB()
{
    database.saveDatabase();
}

void DatabaseWidget::lockDB()
{
    closeDbConnection();
    emit ToUnlockScreen(database.getDatabaseName());
}

void DatabaseWidget::saveDBas()
{
    QString copiedDbPath = QFileDialog::getSaveFileName(0, "Save database as", "copy");
    if (!copiedDbPath.isEmpty())
    {
        if (QFile::copy(database.getDatabaseName(), copiedDbPath + ".db"))
        {
            qDebug() << "Database file copied successfully to:" << copiedDbPath;
        } else
        {
            qDebug() << "Failed to copy database file:" << QFile::FileError();
        }
    } else {
        qDebug() << "No destination directory selected";
    }
}

void DatabaseWidget::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(0, tr("Open File"), "d:/", tr("*.db"));
    if (fileName != "")
    {
        closeDbConnection();
        emit openDatabaseFile(fileName);
        emit AddPath(fileName);
    }
}

void DatabaseWidget::openDownloadedFile(QString fileName)
{
    if (fileName != "")
    {
        closeDbConnection();
        emit openDatabaseFile(fileName);
        QFileInfo file(fileName);
        emit AddPath(file.fileName());
    }
}

void DatabaseWidget::mousePressEvent(QMouseEvent *event)
{
    if (ui->tableView->hasFocus())
    {
         if (event->button() == Qt::RightButton) {
            QPoint pos = event->pos();
            QPoint globalPos = ui->tableView->mapToGlobal(pos);
            QModelIndex index = ui->tableView->indexAt(pos);
            emit tableContextMenu(globalPos, index);
         }
    }

     QWidget::mousePressEvent(event);
}

void DatabaseWidget::copyUsername()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedIndexes();
    if (!selection.empty())
    {
        QModelIndex idIndex = selection.at(0);
        QString username = idIndex.sibling(idIndex.row(), 3).data().toString();
        pclipboard->setText(username);
    }
}

void DatabaseWidget::copyPassword()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedIndexes();
    if (!selection.empty())
    {
        QModelIndex idIndex = selection.at(0);
        QString password = idIndex.sibling(idIndex.row(), 4).data().toString();
        pclipboard->setText(password);
    }
}

void DatabaseWidget::copyNotes()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedIndexes();
    if (!selection.empty())
    {
        QModelIndex idIndex = selection.at(0);
        QString notes = idIndex.sibling(idIndex.row(), 6).data().toString();
        pclipboard->setText(notes);
    }
}

void DatabaseWidget::copyTitle()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedIndexes();
    if (!selection.empty())
    {
        QModelIndex idIndex = selection.at(0);
        QString title = idIndex.sibling(idIndex.row(), 2).data().toString();
        pclipboard->setText(title);
    }
}

void DatabaseWidget::copyUrl()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedIndexes();
    if (!selection.empty())
    {
        QModelIndex idIndex = selection.at(0);
        QString url = idIndex.sibling(idIndex.row(), 5).data().toString();
        pclipboard->setText(url);
    }
}

void DatabaseWidget::on_pushButton_all_clicked()
{
    database.getModel()->setFilter("");
    database.getModel()->select();
}


void DatabaseWidget::on_pushButton_weak_clicked()
{
    database.getModel()->setFilter("entropy < 80");
    database.getModel()->select();
}

void DatabaseWidget::updateDropboxRefToken(QString refresh_token)
{
    database.updateRefreshToken(refresh_token);
}

void DatabaseWidget::requestToUploadToDropbox()
{
    dropbox->authorizeRequest(database.getRefreshToken());
    lastDropboxActions = UploadCurrent;
}

void DatabaseWidget::requestToDownloadFromDropbox()
{
    dropbox->authorizeRequest(database.getRefreshToken());
    lastDropboxActions = Download;
}

void DatabaseWidget::requestToSyncWithDropbox()
{
    dropbox->authorizeRequest(database.getRefreshToken());
    lastDropboxActions = Sync;
}

void DatabaseWidget::showAuthDialog()
{
    authCodeDialog->show();
}

void DatabaseWidget::startDropboxActionFlow()
{
    if(lastDropboxActions == UploadCurrent)
    {
        dropbox->uploadFile(databasePath);
    }
    else if (lastDropboxActions == UploadSpecified)
    {

    }
    else if (lastDropboxActions == Download)
    {
        dropbox->DownloadFile();
    }
    else if (lastDropboxActions == Sync)
    {
        QFileInfo fileInfo(databasePath);
        dropbox->Syncronize(fileInfo.fileName());
    }
}

void DatabaseWidget::synchronize()
{
    emit getPasswordToDatabase("Please, enter your password again to synchronize database with dropbox.");
}

void DatabaseWidget::ContinueSynchronize(QString password)
{
    QFileInfo file(databasePath);
    if(database.synchronizeDatabases(file.fileName(), password))
    {
        dropbox->uploadFileWithOverwriteMode();
    }
    else
    {
        QMessageBox::critical(nullptr, "Error", "An error occurred while trying to synchronize the databases.", QMessageBox::Ok);
    }
}


void DatabaseWidget::on_RepeatedPasswordButton_clicked()
{
    database.filterIdenticalPasswords();
}

