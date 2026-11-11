#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QFile>
#include <QPointer>
#include <QString>

#include "storeapi.h"

// Downloads one file at a time through StoreApi into the download folder and
// opens the installer when it is done.
class Downloader : public QObject
{
    Q_OBJECT
public:
    // downloadError() codes
    enum ErrorCode {
        ConnectionError = 0, // network/HTTP error, timeout, too many redirects
        NoSpaceError = 1,    // no drive with enough free space
        WriteError = 2,      // the file could not be created or written
        InstallerError = 3   // the download is done, but the installer could not be started
    };

    explicit Downloader(StoreApi *api, QObject *parent = 0);

    // App download. source is a full URL or a file name under
    // /static/content/files/ on the server. size is the expected size in
    // bytes (used for the free space check). id and language go to the
    // download statistics.
    void startDownload(const QString &source, qint64 size, const QString &id, const QString &language);
    // Self-update download: no statistics, no size known.
    void startDownloadWithoutStats(const QString &source);

    bool cancelDownload();

    bool deleteFile();

    // "<drive>/download/Wunderland" on F:, E: or C:/Data (in this order),
    // the first one that exists, can be created and has more than minFree
    // bytes free. Empty if none.
    static QString downloadFolder(qint64 minFree = 0);

signals:
    void downloadDone();
    void downloadError(int code); // an ErrorCode
    void progressChanged(int percent);

private slots:
    void onProgress(qint64 received, qint64 total);
    void onFinished();
    void onStatsFinished();

private:
    bool start(const QString &source, qint64 size);

    StoreApi *m_api;
    QPointer<ApiReply> m_reply;
    QFile m_file;
};

#endif // DOWNLOADER_H
