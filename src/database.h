#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlRelationalTableModel>

class Database
{
public:
    Database();
    bool createConnection(QString path, QString passphrase);
    bool initDatabase();
    bool initModel();
    QSqlRelationalTableModel* getModel();
    const QString getDatabaseName();
    bool insertRecord(QSqlRecord record);
    bool updateRecord(QSqlRecord record, int row);
    bool removeRecord(QModelIndex index);
    bool insertGroup(QSqlRecord record);
    bool updateGroup(QSqlRecord record, int row);
    bool removeGroup(QModelIndex index);
    bool closeDatabase();
    bool saveDatabase();
    bool updateRefreshToken(QString refresh_token);
    bool isOpen();
    QString getRefreshToken();
    bool synchronizeDatabases(QString db_name, QString password);
    bool filterIdenticalPasswords();

private:
    QSqlDatabase db;
    QSqlDatabase syncdb;
    QSqlRelationalTableModel *model;
    bool sync_mode;

    bool synchronizeEntries(QSqlDatabase &fromDb, QSqlDatabase &toDb);
    bool synchronizeGroups(QSqlDatabase &fromDb, QSqlDatabase &toDb);
};

#endif // DATABASE_H
