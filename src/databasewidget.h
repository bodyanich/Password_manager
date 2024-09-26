#ifndef DATABASEWIDGET_H
#define DATABASEWIDGET_H

#include <QWidget>
#include "database.h"
#include "authcodedialog.h"
#include "Dropbox/dropboxapi.h"
#include <QSqlRecord>
#include <QItemSelection>
#include <QStyledItemDelegate>
#include <QPainter>

namespace Ui {
class DatabaseWidget;
}

class DatabaseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DatabaseWidget(QWidget *parent = nullptr);
    ~DatabaseWidget();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void on_pass_button_clicked();
    void tryToUnlock(const QString file, const QString passphrase);
    void createNewDatabase(QString file_path, QString passphrase, QString name);
    void addEntry(QSqlRecord record);
    void UpdateEntry(QSqlRecord record, int row);
    void addGroup(QSqlRecord record);
    void UpdateGroup(QSqlRecord record, int row);
    void listViewItemChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void tableViewItemChanged(const QItemSelection & selected, const QItemSelection & deselected);

    void on_pushButton_all_clicked();

    void on_pushButton_weak_clicked();

    void on_RepeatedPasswordButton_clicked();

public slots:
    void needDataToAddEntry();
    void needDataToUpdateEntry();
    void removeEntry();
    void needDataToUpdateGroup();
    void removeGroup();
    void tableItemActivated(const QModelIndex &index);
    void listItemActivated(const QModelIndex &index);
    void closeDbConnection();
    void openFile();
    void openDownloadedFile(QString filename);
    void saveDB();
    void lockDB();
    void saveDBas();

    void requestToUploadToDropbox();
    void requestToDownloadFromDropbox();
    void requestToSyncWithDropbox();
    void synchronize();
    void ContinueSynchronize(QString password);

    void startDropboxActionFlow();
    void updateDropboxRefToken(QString refresh_token);
    void showAuthDialog();

    void copyUsername();
    void copyPassword();
    void copyNotes();
    void copyTitle();
    void copyUrl();


signals:
    void switchToDatabaseScreenRequest();
    void addEntryToDatabase(bool m, int group_id, QString group_name); // 0 - update, 1 - add
    void updateEntryInDatabase(bool m, int row, QSqlRecord r, QString group_name); // 0 - update, 1 - add
    void updateGroupInDatabase(bool m, int row, QSqlRecord r, QString group_name); // 0 - update, 1 - add
    void newDatabasePath(const QString path);
    void openDatabaseFile(QString);
    void AddPath(const QString& fileName);
    void ToUnlockScreen(const QString path);
    void tableContextMenu(const QPoint gpos, const QModelIndex index);
    void getPasswordToDatabase(const QString str);

public:
    enum DropboxActions
    {
        UploadCurrent,
        UploadSpecified,
        Download,
        Sync,
        None
    };

private:
    Ui::DatabaseWidget *ui;
    Database database;
    AuthCodeDialog* authCodeDialog;
    bool pass_mode = true;
    QPixmap pass_mode_view;
    QPixmap pass_mode_hide;
    QString group_name;
    QString record_title;
    QClipboard* pclipboard;
    DropboxApi* dropbox;
    QString databasePath;
    DropboxActions lastDropboxActions;

    void initViews();
    void ChangeButtonPassMode();
    void adjustHeader();
    void customizeTableView();
    void customizeListView();

};

class CustomItemDelegate : public QStyledItemDelegate
{
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        // Set background color for even rows to light gray
        if (index.row() % 2 == 0)
        {
            painter->fillRect(option.rect, QColor(39, 39, 39));
        }
        // Set background color for odd rows to white
        else
        {
            painter->fillRect(option.rect, QColor(45, 45, 45));
        }

        if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, QColor(0, 0, 255, 50)); // Set blue background color with 50% opacity when selected
        }

        // Set font for cells
        QFont font = painter->font();
        font.setPointSize(10);
        painter->setFont(font);

        QRect rect = option.rect.adjusted(10, 0, 0, 0);
        painter->drawText(rect, Qt::AlignLeft, index.data(Qt::DisplayRole).toString());
    }
};

#endif // DATABASEWIDGET_H
