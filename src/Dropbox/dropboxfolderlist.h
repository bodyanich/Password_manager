#ifndef DROPBOXFOLDERLIST_H
#define DROPBOXFOLDERLIST_H

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
class DropboxFolderList;
}

class DropboxFolderList : public QWidget
{
    Q_OBJECT

public:
    explicit DropboxFolderList(QWidget *parent = nullptr);
    ~DropboxFolderList();
    void FillListWidget(QStringList list);

signals:
    void DatabaseNameToDownload(QString);

private slots:
    void ParseListItemActivated(QListWidgetItem* item);

private:
    Ui::DropboxFolderList *ui;
};

#endif // DROPBOXFOLDERLIST_H
