#include "database.h"
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QDateTime>
#include <QSqlRecord>
#include <QFile>

Database::Database()
{
    db = QSqlDatabase::addDatabase("QSQLCIPHER", "main db");
    syncdb = QSqlDatabase::addDatabase("QSQLCIPHER", "sync db");
    sync_mode = false;
}

bool Database::createConnection(QString path, QString passphrase)
{
    QSqlQuery* query;
    if (sync_mode)
    {
        qDebug() << "connecting...";
        syncdb.setDatabaseName(path);
        if (!syncdb.open())
        {
            QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
                                  QObject::tr("Unable to establish a database connection.\n"
                                              "This example needs SQLite support. Please read "
                                              "the Qt SQL driver documentation for information how "
                                              "to build it.\n\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
            return false;
        }
        qDebug() << "connected";
        query = new QSqlQuery(syncdb);
    }
    else
    {
        qDebug() << "connecting to db...";
        db.setDatabaseName(path);
        if (!db.open())
        {
            QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
                                  QObject::tr("Unable to establish a database connection.\n"
                                              "This example needs SQLite support. Please read "
                                              "the Qt SQL driver documentation for information how "
                                              "to build it.\n\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
            return false;
        }
        qDebug() << "connected";
        query = new QSqlQuery(db);
    }

    query->prepare("PRAGMA key = " + passphrase + ";");
    if (!query->exec())
    {
        qDebug() << "Error pragma key: " << query->lastError().text();
        return false;
    }

    qDebug() << "pragma keyed";

    query->prepare("SELECT count(*) FROM sqlite_master;");
    if (!query->exec())
    {
        qDebug() << "Error SELECT count(*) FROM sqlite_master; ----" << query->lastError().text();
        return false;
    }

    qDebug() << "selected";

    delete query;
    return true;
}

bool Database::initDatabase()
{
    QSqlQuery query(db);

    query.prepare("CREATE TABLE IF NOT EXISTS entries (id INTEGER PRIMARY KEY, "
                "group_id INTEGER, "
                "title TEXT, "
                "username TEXT,"
                "password TEXT,"
                "url TEXT, "
                "notes TEXT, "
                "created DECIMAL,"
                "modified DECIMAL,"
                "entropy DECIMAL)");
    if (!query.exec())
    {
        qDebug() << "Error creating table entries: " << query.lastError().text();
        return false;
    }

    query.prepare("CREATE TABLE IF NOT EXISTS groups (id INTEGER PRIMARY KEY, "
               "name TEXT, "
               "notes TEXT,"
               "created DECIMAL,"
               "modified DECIMAL)");
    if (!query.exec())
    {
        qDebug() << "Error creating table groups" << query.lastError().text();
        return false;
    }

    query.prepare("CREATE TABLE IF NOT EXISTS tokens (id INTEGER PRIMARY KEY, "
                  "refresh_token TEXT)");
    if (!query.exec())
    {
        qDebug() << "Error creating table tokens" << query.lastError().text();
        return false;
    }

    query.prepare("INSERT INTO tokens(id, refresh_token) "
                  "VALUES(:id, :refresh_token)");
    query.bindValue(":id", 0);
    query.bindValue(":refresh_token", "");
    if (!query.exec())
    {
        qDebug() << "Error inserting into groups : " << query.lastError().text();
        return false;
    }

    query.prepare("INSERT INTO groups(id, name, notes, created, modified) "
                  "VALUES(:id, :name, :notes, :created, :modified)");
    query.bindValue(":id", 0);
    query.bindValue(":name", "General");
    query.bindValue(":notes", "");
    query.bindValue(":created", QDateTime::currentDateTime());
    query.bindValue(":modified", QDateTime::currentDateTime());
    if (!query.exec())
    {
        qDebug() << "Error inserting into groups : " << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::initModel()
{
    model = new QSqlRelationalTableModel(nullptr, db);
    model->setTable("entries");
    model->setRelation(1, QSqlRelation("groups", "id", "name"));
    model->setFilter("group_id = 0") ;
    model->select();
    return true;
}

bool Database::closeDatabase()
{
    model->revertAll();
    model->submitAll();
    model = nullptr;
    db.close();
    return true;
}

bool Database::isOpen()
{
    return db.isOpen();
}

bool Database::saveDatabase()
{
    if (model->submitAll())
        return true;

    return false;
}

QSqlRelationalTableModel* Database::getModel()
{
    return model;
}

const QString Database::getDatabaseName()
{
    return db.databaseName();
}

bool Database::insertRecord(QSqlRecord record)
{
    model->insertRecord(-1, record);
    return true;
}

bool Database::updateRecord(QSqlRecord record, int row)
{
    model->setRecord(row, record);
    return true;
}

bool Database::removeRecord(QModelIndex index)
{
    model->removeRow(index.row());
    model->submitAll();
    model->select();
    return true;
}

bool Database::insertGroup(QSqlRecord record)
{
    model->relationModel(1)->insertRecord(-1, record);
    return true;
}

bool Database::updateGroup(QSqlRecord record, int row)
{
    model->relationModel(1)->setRecord(row, record);
    return true;
}

bool Database::removeGroup(QModelIndex index)
{
    model->relationModel(1)->removeRow(index.row());
    model->relationModel(1)->submitAll();
    model->relationModel(1)->select();
    return true;
}

bool Database::updateRefreshToken(QString refresh_token)
{
    QSqlQuery query(db);

    query.prepare("UPDATE tokens SET refresh_token = :refresh_token WHERE id = :id");

    query.bindValue(":id", 0);
    query.bindValue(":refresh_token", refresh_token);

    if (!query.exec()) {
        qDebug() << "Error updating token: " << query.lastError().text();
        return false;
    } else {
        qDebug() << "Token updated successfully.";
    }
}

QString Database::getRefreshToken()
{
    QSqlQuery query(db);

    // Prepare the select query
    query.prepare("SELECT (refresh_token) FROM tokens WHERE id = (:id)");

    // Bind the id to the parameter
    query.bindValue(":id", 0);

    // Execute the query
    if (!query.exec()) {
        qDebug() << "Error retrieving token: " << query.lastError().text();
        return QString();
    }

    // Check if there is a result
    if (query.next()) {
        // Retrieve the refresh_token value
        QString refresh_token = query.value(0).toString();
        return refresh_token;
    } else {
        qDebug() << "No token found for id 0.";
        return QString(); // Return an empty QString if no token found
    }
}

bool Database::synchronizeDatabases(QString db_name, QString password)
{
    sync_mode = true;
    createConnection(db_name, password);
    sync_mode = false;

    if (synchronizeEntries(db, syncdb) && synchronizeEntries(syncdb, db) &&
        synchronizeGroups(db, syncdb) && synchronizeGroups(syncdb, db))
    {
        syncdb.close();
        saveDatabase();
        model->select();
        return true;
    }

    syncdb.close();
    return false;
}

bool Database::synchronizeEntries(QSqlDatabase &fromDb, QSqlDatabase &toDb)
{
    // Retrieve records from the source database in chunks
    const int chunkSize = 100; // Adjust as needed
    int offset = 0;
    while (true) {
        QSqlQuery query(fromDb);
        query.prepare("SELECT * FROM entries LIMIT :chunkSize OFFSET :offset");
        query.bindValue(":chunkSize", chunkSize);
        query.bindValue(":offset", offset);

        if (!query.exec()) {
            qDebug() << "Failed to select records from source database:" << query.lastError().text();
            return false;
        }

        int rowCount = 0;
        // Compare records between the source and destination databases
        while (query.next()) {
            ++rowCount;
            int id = query.value("id").toInt();
            // Check if the record exists in the destination database
            QSqlQuery checkQuery(toDb);
            checkQuery.prepare("SELECT id, modified FROM entries WHERE id = :id");
            checkQuery.bindValue(":id", id);

            if (!checkQuery.exec()) {
                qDebug() << "Failed to check record existence in destination database:" << checkQuery.lastError().text();
                return false;
            }

            // Insert or update records based on the last modified timestamp
            if (checkQuery.next()) {
                // Record exists in the destination database
                // Check if modified timestamp is greater in the source
                if (query.value("modified").toDateTime() > checkQuery.value("modified").toDateTime()) {
                    // Update the record in the destination database
                    QSqlQuery updateQuery(toDb);
                    updateQuery.prepare("UPDATE entries SET group_id = :group_id, title = :title, username = :username, password = :password, "
                                        "url = :url, notes = :notes, created = :created, modified = :modified, entropy = :entropy WHERE id = :id");
                    // Bind values from source query to destination query
                    // (Modify this part according to your table structure)
                    updateQuery.bindValue(":group_id", query.value("group_id"));
                    updateQuery.bindValue(":title", query.value("title"));
                    updateQuery.bindValue(":username", query.value("username"));
                    updateQuery.bindValue(":password", query.value("password"));
                    updateQuery.bindValue(":url", query.value("url"));
                    updateQuery.bindValue(":notes", query.value("notes"));
                    updateQuery.bindValue(":created", query.value("created"));
                    updateQuery.bindValue(":modified", query.value("modified"));
                    updateQuery.bindValue(":entropy", query.value("entropy"));
                    updateQuery.bindValue(":id", id);
                    if (!updateQuery.exec()) {
                        qDebug() << "Failed to update record in destination database:" << updateQuery.lastError().text();
                        return false;
                    }
                }
            } else {
                // Record does not exist in the destination database, insert it
                QSqlQuery insertQuery(toDb);
                insertQuery.prepare("INSERT INTO entries (id, group_id, title, username, password, url, notes, created, modified, entropy) "
                                    "VALUES (:id, :group_id, :title, :username, :password, :url, :notes, :created, :modified, :entropy)");
                // Bind values from source query to destination query
                // (Modify this part according to your table structure)
                insertQuery.bindValue(":id", query.value("id"));
                insertQuery.bindValue(":group_id", query.value("group_id"));
                insertQuery.bindValue(":title", query.value("title"));
                insertQuery.bindValue(":username", query.value("username"));
                insertQuery.bindValue(":password", query.value("password"));
                insertQuery.bindValue(":url", query.value("url"));
                insertQuery.bindValue(":notes", query.value("notes"));
                insertQuery.bindValue(":created", query.value("created"));
                insertQuery.bindValue(":modified", query.value("modified"));
                insertQuery.bindValue(":entropy", query.value("entropy"));
                if (!insertQuery.exec()) {
                    qDebug() << "Failed to insert record into destination database:" << insertQuery.lastError().text();
                    return false;
                }
            }
        }
        // Break loop if no more records
        if (rowCount < chunkSize) break;
        // Update offset for the next chunk
        offset += chunkSize;
    }

    qDebug() << "Synchronization of entries completed.";
    return true;
}

bool Database::synchronizeGroups(QSqlDatabase &fromDb, QSqlDatabase &toDb)
{
    // Retrieve records from the source database in chunks
    const int chunkSize = 100; // Adjust as needed
    int offset = 0;
    while (true) {
        QSqlQuery query(fromDb);
        query.prepare("SELECT * FROM groups LIMIT :chunkSize OFFSET :offset");
        query.bindValue(":chunkSize", chunkSize);
        query.bindValue(":offset", offset);
        if (!query.exec()) {
            qDebug() << "Failed to select records from source groups table:" << query.lastError().text();
            return false;
        }

        int rowCount = 0;
        // Compare records between the source and destination groups tables
        while (query.next()) {
            ++rowCount;
            int id = query.value("id").toInt();
            // Check if the record exists in the destination groups table
            QSqlQuery checkQuery(toDb);
            checkQuery.prepare("SELECT id, modified FROM groups WHERE id = :id");
            checkQuery.bindValue(":id", id);
            if (!checkQuery.exec()) {
                qDebug() << "Failed to check record existence in destination groups table:" << checkQuery.lastError().text();
                return false;
            }

            // Insert or update records based on the last modified timestamp
            if (checkQuery.next()) {
                // Record exists in the destination groups table
                // Check if modified timestamp is greater in the source
                if (query.value("modified").toDateTime() > checkQuery.value("modified").toDateTime()) {
                    // Update the record in the destination groups table
                    QSqlQuery updateQuery(toDb);
                    updateQuery.prepare("UPDATE groups SET name = :name, notes = :notes, created = :created, "
                                        "modified = :modified WHERE id = :id");
                    // Bind values from source query to destination query
                    // (Modify this part according to your table structure)
                    updateQuery.bindValue(":name", query.value("name"));
                    updateQuery.bindValue(":notes", query.value("notes"));
                    updateQuery.bindValue(":created", query.value("created"));
                    updateQuery.bindValue(":modified", query.value("modified"));
                    updateQuery.bindValue(":id", id);
                    if (!updateQuery.exec()) {
                        qDebug() << "Failed to update record in destination groups table:" << updateQuery.lastError().text();
                        return false;
                    }
                }
            } else {
                // Record does not exist in the destination groups table, insert it
                QSqlQuery insertQuery(toDb);
                insertQuery.prepare("INSERT INTO groups (id, name, notes, created, modified) "
                                    "VALUES (:id, :name, :notes, :created, :modified)");
                // Bind values from source query to destination query
                // (Modify this part according to your table structure)
                insertQuery.bindValue(":id", id);
                insertQuery.bindValue(":name", query.value("name"));
                insertQuery.bindValue(":notes", query.value("notes"));
                insertQuery.bindValue(":created", query.value("created"));
                insertQuery.bindValue(":modified", query.value("modified"));
                if (!insertQuery.exec()) {
                    qDebug() << "Failed to insert record into destination groups table:" << insertQuery.lastError().text();
                    return false;
                }
            }
        }
        // Break loop if no more records
        if (rowCount < chunkSize) break;
        // Update offset for the next chunk
        offset += chunkSize;
    }

    qDebug() << "Synchronization of groups completed.";
    return true;
}

bool Database::filterIdenticalPasswords()
{
    QSqlQuery query(db);

    // Query to find passwords that are used more than once
    query.prepare("SELECT password FROM entries GROUP BY password HAVING COUNT(password) > 1");

    if (!query.exec()) {
        qDebug() << "Error finding identical passwords: " << query.lastError().text();
        return false;
    }

    // Collect all passwords that appear more than once
    QStringList identicalPasswords;
    while (query.next()) {
        identicalPasswords << query.value(0).toString();
    }

    // Create a filter string to match entries with identical passwords
    QString filter = "password IN ('" + identicalPasswords.join("','") + "')";

    // Apply the filter to the model
    model->setFilter(filter);
    model->select();

    return true;
}
