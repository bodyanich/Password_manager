#include "welcomewidget.h"
#include "ui_welcomewidget.h"
#include <QFileDialog>
#include <QDebug>
#include <QKeyEvent>
#include <QMessageBox>

WelcomeWidget::WelcomeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WelcomeWidget)
{
    ui->setupUi(this);
    file.setFileName("RecentDatabases.txt");
    refreshLastDatabases();

    QPixmap pixmap(":/Icons/logo2.png");
    ui->iconLabel->setPixmap(pixmap);
    ui->iconLabel->setFixedSize(128, 128);

    connect(ui->buttonNewDatabase, SIGNAL(clicked()), SIGNAL(newDatabase()));
    connect(ui->recentListWidget,
                SIGNAL(itemActivated(QListWidgetItem*)),
                this,
                SLOT(openDatabaseFromFile(QListWidgetItem*)));
}

WelcomeWidget::~WelcomeWidget()
{
    delete ui;
}

void WelcomeWidget::openDatabaseFromFile(QListWidgetItem* item)
{

    if (!item || item->text().isEmpty()) {
        return;
    }
    else
    {
        QFile file(item->text());
        if (!file.exists())
        {
            QMessageBox::critical(nullptr, "Error", "This database doesn't exist now.", QMessageBox::Ok);
        }
        else
        {
            emit openDatabaseFile(item->text());
        }
    }
}

void WelcomeWidget::removeFromLastDatabases(QListWidgetItem* item)
{
    QString file_path = item->text();
    if (!item || item->text().isEmpty())
    {
        return;
    }

    if (file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream in(&file);
        QStringList lines;
        while (!in.atEnd()) {
            QString line = in.readLine();
            lines.append(line);
        }

        lines.removeAll(file_path); // remove the line to be deleted

        file.resize(0);
        file.seek(0);
        QTextStream out(&file);
        for (const QString &line : lines) {
            out << line << '\n'; // write the remaining lines back to the file
        }

        file.close();
    }
    refreshLastDatabases();
}

void WelcomeWidget::refreshLastDatabases()
{
    ui->recentListWidget->clear();
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            ui->recentListWidget->addItem(line);
        }
        file.close();
    }
    else
        qDebug() << "Error with file refreshing.";

}

void WelcomeWidget::addNewDatabasePath(QString path)
{
    if(path.isEmpty())
        return;

    if (file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream out(&file);
        QByteArray data = file.readAll();
        file.seek(0);
        out << path + "\n";
        out << data;
        file.close();
        refreshLastDatabases();
    }
    else
        qDebug() << "Error with file adding new database path.";
}

void WelcomeWidget::on_buttonOpenDatabase_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(0, tr("Open File"), "d:/", tr("*.db"));
    if (fileName != "")
    {
        emit openDatabaseFile(fileName);
    }

    checkIfExistsAndAddPath(fileName);

}

void WelcomeWidget::checkIfExistsAndAddPath(const QString& fileName)
{
    if (file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if(line == fileName)
            {
                file.close();
                return;
            }
        }
        file.close();
        addNewDatabasePath(fileName);
    }
    else
        qDebug() << "Error with file on button clicked.";
}

void WelcomeWidget::keyPressEvent(QKeyEvent* event)
{
    if (ui->recentListWidget->hasFocus()) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        {
            openDatabaseFromFile(ui->recentListWidget->currentItem());
        } else if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
        {
            removeFromLastDatabases(ui->recentListWidget->currentItem());
        }
    }

    QWidget::keyPressEvent(event);
}

void WelcomeWidget::showEvent(QShowEvent* event)
{
    refreshLastDatabases();
    QWidget::showEvent(event);
}

