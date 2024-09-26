#include "dropboxapi.h"
#include <QDesktopServices>
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include "passwordgeneratorcore.h"
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QJsonArray>
#include <QFileInfo>
#include <QFileDialog>

DropboxApi::DropboxApi(QObject* parent): QObject(parent),
    authenticateUrl("https://www.dropbox.com/oauth2/authorize"),
    accessTokenUrl("https://api.dropboxapi.com/oauth2/token"),
    clientIdentifier("1wsebira2gn4php"),
    codeChallengeMethod("S256")
{
    manager = new QNetworkAccessManager();
    refreshManager = new QNetworkAccessManager();
    ToughPassFolderManager = new QNetworkAccessManager();
    checkFileExistingManager = new QNetworkAccessManager();
    uploadManager = new QNetworkAccessManager();
    listFolderManager = new QNetworkAccessManager();
    downloadManager = new QNetworkAccessManager();
    uploadDbDialog = new UploadDbDialog();
    dropboxFolderList = new DropboxFolderList();
    pathToToughPassFolder = "";

    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(AccessTokenManagerFinished(QNetworkReply*)));
    QObject::connect(refreshManager, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(RefreshManagerFinished(QNetworkReply*)));
    QObject::connect(ToughPassFolderManager, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(ToughPassFolderManagerFinished(QNetworkReply*)));
    QObject::connect(checkFileExistingManager, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(checkFileExistingManagerFinished(QNetworkReply*)));
    QObject::connect(uploadManager, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(UploadManagerFinished(QNetworkReply*)));
    QObject::connect(listFolderManager, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(ListDropboxFolderManagerFinished(QNetworkReply*)));
    QObject::connect(downloadManager, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(DownloadManagerFinished(QNetworkReply*)));

    QObject::connect(uploadDbDialog, SIGNAL(confirmUpload(QString)),
                     this, SLOT(continueUploadFile(QString)));
    QObject::connect(dropboxFolderList, SIGNAL(DatabaseNameToDownload(QString)),
                     this, SLOT(continueDownloadFile(QString)));
}

DropboxApi::~DropboxApi()
{
    delete manager;
    delete refreshManager;
    delete ToughPassFolderManager;
    delete checkFileExistingManager;
    delete uploadDbDialog;
    delete uploadManager;
}

void DropboxApi::authorizeRequest(QString ref_token)
{
    if (!ref_token.isEmpty())
    {
        getAccessTokenViaRefresh(ref_token);
    }
    else
    {
        getAuthCodeViaBrowser();
    }
}

void DropboxApi::getAuthCodeViaBrowser()
{
    QString CHALLENGE = makeCodeChallenge();
    QString url = authenticateUrl +
                  "?client_id=" + clientIdentifier +
                  "&token_access_type=offline&response_type=code&code_challenge=" + CHALLENGE +
                  "&code_challenge_method=" + codeChallengeMethod;

    QDesktopServices::openUrl(QUrl(url));
    emit OpenAuthDialog();
}

QString DropboxApi::generateCodeVerifier()
{
    PasswordGenerator::CharClasses classes;
    PasswordGenerator::GeneratorFlags flags;
    PasswordGenerator generator;

    classes = PasswordGenerator::LowerLetters | PasswordGenerator::UpperLetters | PasswordGenerator::Numbers;
    flags |= PasswordGenerator::ExcludeLookAlike;
    generator.setCustomCharacterSet("_-.~");
    generator.setCharClasses(classes);
    generator.setFlags(flags);
    generator.setLength(128);

    code_verifier = generator.generatePassword();
    qDebug() << "code_verifier - " << code_verifier;

    return code_verifier;
}

QString DropboxApi::makeCodeChallenge()
{
    QByteArray codeVerifierBytes = generateCodeVerifier().toUtf8();
    QByteArray hash = QCryptographicHash::hash(codeVerifierBytes, QCryptographicHash::Sha256);

    qDebug() << "hashArray - " << base64Encode(hash);
    return base64Encode(hash);
}

QString DropboxApi::base64Encode(const QByteArray &bytes) {
    QString base64 = bytes.toBase64();
    base64.replace("+", "-");
    base64.replace("/", "_");
    base64.remove("=");
    return base64;
}

void DropboxApi::getAccessToken(QString authCode)
{
    QUrl apiUrl(accessTokenUrl);
    QUrlQuery params;
    params.addQueryItem("code", authCode);
    params.addQueryItem("grant_type", "authorization_code");
    params.addQueryItem("code_verifier", code_verifier);
    params.addQueryItem("client_id", clientIdentifier);

    QNetworkRequest request(apiUrl);
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    request.setSslConfiguration(config);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    manager->post(request, params.toString(QUrl::FullyEncoded).toUtf8());
}

void DropboxApi::AccessTokenManagerFinished(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();

        // Parse JSON response
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        if (!jsonDoc.isNull()) {
            QJsonObject jsonObj = jsonDoc.object();
            access_token = jsonObj["access_token"].toString();
            expires_in = jsonObj["expires_in"].toInt();
            QString refresh_token = jsonObj["refresh_token"].toString();
            emit RefreshTokenWrite(refresh_token);
            emit SuccessfulAuth();
        } else {
            qDebug() << "Failed to parse JSON response";
            QMessageBox::critical(nullptr, "JSON failure", "Failed to parse JSON response.", QMessageBox::Ok);
        }
    } else {
        qDebug() << "Error:" << reply->errorString();
        QMessageBox::critical(nullptr, "Access token request error.", reply->errorString(), QMessageBox::Ok);
    }

    // Clean up
    reply->deleteLater();
}

void DropboxApi::getAccessTokenViaRefresh(QString ref_token)
{
    QUrl apiUrl(accessTokenUrl);
    QUrlQuery params;
    params.addQueryItem("grant_type", "refresh_token");
    params.addQueryItem("refresh_token", ref_token);
    params.addQueryItem("client_id", clientIdentifier);

    QNetworkRequest request(apiUrl);
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    request.setSslConfiguration(config);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    refreshManager->post(request, params.toString(QUrl::FullyEncoded).toUtf8());
}

void DropboxApi::RefreshManagerFinished(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();

        // Parse JSON response
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        if (!jsonDoc.isNull()) {
            QJsonObject jsonObj = jsonDoc.object();
            access_token = jsonObj["access_token"].toString();
            qDebug() << "access token via refresh: " << access_token;
            expires_in = jsonObj["expires_in"].toInt();
            emit SuccessfulAuth();
        } else {
            qDebug() << "Failed to parse JSON response";
            QMessageBox::critical(nullptr, "JSON failure", "Failed to parse JSON response.", QMessageBox::Ok);
        }
    } else {
        //qDebug() << "Error:" << reply->errorString();
        //QMessageBox::critical(nullptr, "Access token via refresh request error.", reply->errorString(), QMessageBox::Ok);
        getAuthCodeViaBrowser();
    }

    // Clean up
    reply->deleteLater();
}

// Function is not used
void DropboxApi::getToughPassFolderPath()
{
    // Construct the JSON object
    QJsonObject jsonObject;
    jsonObject["query"] = "ToughPass";

    // Convert the JSON object to a QByteArray
    QJsonDocument jsonDocument(jsonObject);
    QByteArray jsonData = jsonDocument.toJson();

    QNetworkRequest request;
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    request.setSslConfiguration(config);

    // Set the URL
    request.setUrl(QUrl("https://api.dropboxapi.com/2/files/search_v2"));

    // Set the headers
    request.setRawHeader("Authorization", QString("Bearer %1").arg(access_token).toUtf8());
    qDebug() << "RawHeader: " << QString("Bearer %1").arg(access_token).toUtf8();
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Perform the POST request
    ToughPassFolderManager->post(request, jsonData);
}

void DropboxApi::checkIfFileExistsInToughpass(QString filename)
{
    // Construct the JSON object
    QJsonObject optionsObject;
    optionsObject["filename_only"] = true;

    QJsonObject jsonObject;
    jsonObject["options"] = optionsObject;
    jsonObject["query"] = filename;
    qDebug() << "fileName in checking: " << filename;

    // Convert the JSON object to a QByteArray
    QJsonDocument jsonDocument(jsonObject);
    QByteArray jsonData = jsonDocument.toJson();

    QNetworkRequest request;
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    request.setSslConfiguration(config);

    // Set the URL
    request.setUrl(QUrl("https://api.dropboxapi.com/2/files/search_v2"));

    // Set the headers
    request.setRawHeader("Authorization", QString("Bearer %1").arg(access_token).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Perform the POST request
    checkFileExistingManager->post(request, jsonData);
}

// Function is not used
void DropboxApi::ToughPassFolderManagerFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

        if (!jsonResponse.isNull() && jsonResponse.isObject()) {
            QJsonObject jsonObject = jsonResponse.object();
            if (jsonObject.contains("matches") && jsonObject["matches"].isArray()) {
                QJsonArray matchesArray = jsonObject["matches"].toArray();
                if (!matchesArray.isEmpty()) {
                    QJsonObject matchObject = matchesArray[0].toObject();
                    if (matchObject.contains("metadata") && matchObject["metadata"].isObject()) {
                        QJsonObject metadataObject = matchObject["metadata"].toObject();
                        if (metadataObject.contains("metadata") && metadataObject["metadata"].isObject()) {
                            QJsonObject innerMetadataObject = metadataObject["metadata"].toObject();
                            if (innerMetadataObject.contains("path_lower") && innerMetadataObject["path_lower"].isString()) {
                                pathToToughPassFolder = innerMetadataObject["path_lower"].toString();
                                qDebug() << "Path Lower:" << pathToToughPassFolder;

                                QFileInfo fileInfo(pathToUploadFile);
                                checkIfFileExistsInToughpass(fileInfo.fileName());
                            }
                        }
                    }
                }
            }
        }
    } else {
        qDebug() << "Error:" << reply->errorString();
        QMessageBox::critical(nullptr, "Error getting ToughPass folder path in dropbox", reply->errorString(), QMessageBox::Ok);
    }
    // Clean up
    reply->deleteLater();
}

void DropboxApi::checkFileExistingManagerFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

        if (!jsonResponse.isNull() && jsonResponse.isObject()) {
            QJsonObject jsonObject = jsonResponse.object();
            if (jsonObject.contains("matches") && jsonObject["matches"].isArray()) {
                QJsonArray matchesArray = jsonObject["matches"].toArray();
                if (matchesArray.isEmpty()) {
                    continueUploadFile("add");
                }
                else
                {
                    uploadDbDialog->show();
                }
            }
        }
    } else {
        qDebug() << "Error:" << reply->errorString();
        QMessageBox::critical(nullptr, "Error checking whether database already exists in dropbox", reply->errorString(), QMessageBox::Ok);
    }

    // Clean up
    reply->deleteLater();
}

void DropboxApi::uploadFile(QString file_path)
{
    pathToUploadFile = file_path;
    QFileInfo fileInfo(pathToUploadFile);
    checkIfFileExistsInToughpass(fileInfo.fileName());
}

void DropboxApi::uploadFileWithOverwriteMode()
{
    pathToUploadFile = databaseNameToSync;
    qDebug() << "databaseNameToSync: " << databaseNameToSync;
    continueUploadFile("overwrite");
}

void DropboxApi::continueUploadFile(QString mode)
{
    QFileInfo fileInfo(pathToUploadFile);
    QString filePath = "/" + fileInfo.fileName();
    qDebug() << "filePath in continue upload file: " << filePath;
    QString dropboxApiArg = "{\"path\":\"" + filePath + "\",\"mode\":{\".tag\":\"" + mode + "\"},\"autorename\":true}";

    QFile file(pathToUploadFile);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Failed to open upload database:", file.errorString(), QMessageBox::Ok);
        return;
    }

    // Construct the URL
    QUrl url("https://content.dropboxapi.com/2/files/upload");

    // Construct the request
    QNetworkRequest request(url);
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    request.setSslConfiguration(config);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(access_token).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    request.setRawHeader("Dropbox-API-Arg", dropboxApiArg.toUtf8());

    uploadManager->post(request, file.readAll());
}

void DropboxApi::UploadManagerFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        if (emitFunc == DropboxApi::Sync)
        {
            QMessageBox::information(nullptr, "Success", "The database has been successfully synchronized with dropbox.", QMessageBox::Ok);
            deleteFile(databaseNameToSync);
        }
        else
        {
            QMessageBox::information(nullptr, "Success", "The database has been successfully uploaded to Dropbox.", QMessageBox::Ok);
        }
    }
    else {
        if (emitFunc == DropboxApi::Sync)
        {
            QMessageBox::critical(nullptr, "Failed to upload synchronized database:", reply->errorString(), QMessageBox::Ok);
            deleteFile(databaseNameToSync);
        }
        else
        {
            QMessageBox::critical(nullptr, "Failed to upload database:", reply->errorString(), QMessageBox::Ok);
        }
    }
    // Clean up
    reply->deleteLater();
}

bool  DropboxApi::deleteFile(QString filepath)
{
    QFile file(filepath);
    qDebug() << "filepath" << filepath;

    // Check if the file exists
    if (!file.exists()) {
        qDebug() << "File does not exist.";
        return false; // Exit with error
    }

    // Attempt to remove the file
    if (file.remove())
    {
        qDebug() << "File removed successfully.";
    }
    else
    {
        qDebug() << "Failed to remove file.";
        return false; // Exit with error
    }

    return true; // Exit successfully
}

//Download part

void DropboxApi::DownloadFile()
{
    emitFunc = DropboxApi::Download;
    listDropboxFolder();
}

void DropboxApi::listDropboxFolder()
{
    // Construct the JSON object
    QJsonObject jsonObject;
    jsonObject["path"] = "";
    jsonObject["recursive"] = true;

    // Convert JSON object to byte array
    QByteArray postData = QJsonDocument(jsonObject).toJson();

    // Construct the URL
    QUrl url("https://api.dropboxapi.com/2/files/list_folder");

    // Construct the request
    QNetworkRequest request(url);
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    request.setSslConfiguration(config);

    request.setRawHeader("Authorization", QString("Bearer %1").arg(access_token).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    listFolderManager->post(request, postData);
}

void DropboxApi::ListDropboxFolderManagerFinished(QNetworkReply *reply)
{
    QStringList pathDisplays;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument responseJson = QJsonDocument::fromJson(responseData);
        QJsonObject responseObject = responseJson.object();

        if (responseObject.contains("entries") && responseObject["entries"].isArray()) {
            QJsonArray entriesArray = responseObject["entries"].toArray();
            for (const auto &entry : entriesArray) {
                if (entry.isObject()) {
                    QString pathDisplay = entry.toObject()["path_display"].toString();
                    if (pathDisplay.endsWith(".db")) {
                        pathDisplays.append(pathDisplay);
                    }
                }
            }
        }

        if (emitFunc == DropboxApi::Download)
        {
            dropboxFolderList->FillListWidget(pathDisplays);
            dropboxFolderList->show();
        }
        else if (emitFunc == DropboxApi::Sync)
        {
            if(pathDisplays.contains("/" + databaseNameToSync))
            {
                continueDownloadFile("/" + databaseNameToSync);
            }
            else
            {
                QMessageBox::critical(nullptr, "Error", "Database \"" + databaseNameToSync +
                                                            "\" doesn't exist in dropbox. Synchronization is not possible.", QMessageBox::Ok);
            }
        }
    } else
    {
        qDebug() << "Error:" << reply->errorString();
        QMessageBox::critical(nullptr, "Failed to get list of dropbox databases.", reply->errorString(), QMessageBox::Ok);
    }
    // Clean up
    reply->deleteLater();
}

void  DropboxApi::continueDownloadFile(QString filename)
{
    downloadedFileName = filename;
    // Construct the URL
    QUrl url("https://content.dropboxapi.com/2/files/download");

    // Construct the request
    QNetworkRequest request(url);
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    request.setSslConfiguration(config);

    request.setRawHeader("Authorization", QString("Bearer %1").arg(access_token).toUtf8());
    request.setRawHeader("Dropbox-API-Arg", "{\"path\":\"" + filename.toUtf8() + "\"}");

    downloadManager->post(request, "");
}

void DropboxApi::DownloadManagerFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        saveDownloadedFile(responseData);
    }
    else {
        QMessageBox::critical(nullptr, "Download error", reply->errorString(), QMessageBox::Ok);
        qDebug() << "Error:" << reply->errorString();
    }
    // Clean up
    reply->deleteLater();
}

void DropboxApi::saveDownloadedFile(QByteArray responseData)
{
    QFile file;
    QString dir;
    if (emitFunc == DropboxApi::Download)
    {
        dir = QFileDialog::getExistingDirectory(nullptr, tr("Directory to download database"),
                                                        "/",
                                                        QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);

        file.setFileName(dir + "\\" + downloadedFileName);
    }
    else if (emitFunc == DropboxApi::Sync)
    {
        file.setFileName(databaseNameToSync);
    }

    if (file.exists())
    {
        QMessageBox::critical(nullptr, "Error", "This database already exists in " + dir + " folder.", QMessageBox::Ok);
    }
    else
    {
        if (file.open(QIODevice::ReadWrite))
        {
            qint64 bytesWritten = file.write(responseData);
            if (bytesWritten != responseData.size()) {
                qDebug() << "Failed to write all data to file.";
                QMessageBox::critical(nullptr, "Error", "Failed to write all data to database while saving it.", QMessageBox::Ok);
                qDebug() << "file.fileName: " << file.fileName();
            } else {
                if(emitFunc == DropboxApi::Download)
                {
                    QMessageBox::information(nullptr, "Success", "Database downloaded and saved successfully.", QMessageBox::Ok);
                    emit openDownloadedDatabase(file.fileName());
                }
                else if (emitFunc == DropboxApi::Sync)
                {
                    emit ReadyForSync();
                }
                qDebug() << "File downloaded and saved successfully.";
            }
        }
        else
        {
            QMessageBox::critical(nullptr, "Error", "Failed to open database file for writing.", QMessageBox::Ok);
            qDebug() << "Failed to open file for writing.";
        }
    }

}

void DropboxApi::Syncronize(QString databaseName)
{
    databaseNameToSync = databaseName;
    emitFunc = DropboxApi::Sync;
    listDropboxFolder();
}
