#ifndef DROPBOXAPI_H
#define DROPBOXAPI_H

#include <QNetworkAccessManager>
#include <QString>
#include "uploaddbdialog.h"
#include "dropboxfolderlist.h"

class DropboxApi : public QObject
{
    Q_OBJECT

public:
    explicit DropboxApi(QObject* parent = nullptr);
    ~DropboxApi();

    enum emitFunctions
    {
        Download,
        Sync
    };

    void authorizeRequest(QString ref_token);
    QString generateCodeVerifier();
    QString makeCodeChallenge();
    QString base64Encode(const QByteArray &bytes);
    void getAuthCodeViaBrowser();

    void getToughPassFolderPath();
    void checkIfFileExistsInToughpass(QString filename);
    void uploadFile(QString file_path);
    void uploadFileWithOverwriteMode();

    void DownloadFile();
    void listDropboxFolder();
    void saveDownloadedFile(QByteArray);

    void Syncronize(QString databaseName);

public slots:
    void getAccessToken(QString authCode);
    void getAccessTokenViaRefresh(QString ref_token);
    void continueUploadFile(QString mode);

private slots:
    void AccessTokenManagerFinished(QNetworkReply *reply);
    void RefreshManagerFinished(QNetworkReply *reply);
    void ToughPassFolderManagerFinished(QNetworkReply *reply);
    void checkFileExistingManagerFinished(QNetworkReply *reply);
    void UploadManagerFinished(QNetworkReply *reply);
    void ListDropboxFolderManagerFinished(QNetworkReply *reply);
    void DownloadManagerFinished(QNetworkReply *reply);
    void continueDownloadFile(QString filename);

signals:
    void RefreshTokenWrite(QString refresh_token);
    void OpenAuthDialog();
    void SuccessfulAuth();
    void PickDatabaseForDownload(QStringList list);
    void openDownloadedDatabase(QString filepath);
    void ReadyForSync();

private:
    QNetworkAccessManager* manager;
    QNetworkAccessManager* refreshManager;
    QNetworkAccessManager* ToughPassFolderManager;
    QNetworkAccessManager* checkFileExistingManager;
    QNetworkAccessManager* uploadManager;
    QNetworkAccessManager* listFolderManager;
    QNetworkAccessManager* downloadManager;
    UploadDbDialog* uploadDbDialog;
    DropboxFolderList* dropboxFolderList;
    const QString authenticateUrl;
    const QString accessTokenUrl;
    const QString clientIdentifier;
    const QString codeChallengeMethod;
    QString code_verifier;
    QString access_token;
    int expires_in;
    QString pathToToughPassFolder;
    QString pathToUploadFile;
    QString downloadedFileName;
    QString databaseNameToSync;
    emitFunctions emitFunc;

    bool  deleteFile(QString filepath);

};

#endif // DROPBOXAPI_H
