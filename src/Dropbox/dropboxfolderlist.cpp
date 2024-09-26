#include "dropboxfolderlist.h"
#include "ui_dropboxfolderlist.h"

DropboxFolderList::DropboxFolderList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DropboxFolderList)
{
    ui->setupUi(this);

    connect(ui->listWidget,
            SIGNAL(itemActivated(QListWidgetItem*)),
            this,
            SLOT(ParseListItemActivated(QListWidgetItem*)));
}

DropboxFolderList::~DropboxFolderList()
{
    delete ui;
}

void DropboxFolderList::FillListWidget(QStringList list)
{
    ui->listWidget->clear();
    for(const auto& item: list)
    {
        ui->listWidget->addItem(item);
    }
}

void DropboxFolderList::ParseListItemActivated(QListWidgetItem* item)
{
    if (!item || item->text().isEmpty()) {
        return;
    }
    emit DatabaseNameToDownload(item->text());
    this->close();
    this->destroy();
}
