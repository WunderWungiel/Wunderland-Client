#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSystemStorageInfo>
#include <QDesktopServices>

#include "appmanagement.h"

#include <QDebug>

class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(QObject *parent = 0);
    
    void startDownload(QString _source, long long size, QString id, QString lan, QString os, QString resolution);
    void startDownloadWithoutStats(QString _source);

    bool cancelDownload();

    bool deleteFile();

signals:
    void downloadDone();
    void downloadError(int code); // 0 -> error receive reply from stat server; 1 -> not enough space
    void progressChanged(int percent);
    
public slots:
    void downloadFinished();
    void saveBytes() {
        //qDebug() << "saveBytes";
        file.write(reply->read(reply->bytesAvailable()));
    }
    void progress(qint64 bytesReceived, qint64 bytesTotal) {
        // bytesTotal is -1 (or 0) when the server sends no Content-Length
        if (bytesTotal <= 0) return;
        emit progressChanged((int)(((double)bytesReceived/bytesTotal)*100));
    }
    void startRealDownload();

private:
    QNetworkAccessManager* n_manager;
    QFile file;
    QNetworkReply* reply;
    QNetworkReply* statReply;

    bool downloading, aborted;
    QString sourcePath, source;
    
    // Returns the redirect target of a finished reply, resolved against the request URL.
    // Returns an empty URL if there is no redirect, if it points back to the same URL, or
    // if the hop limit is reached (the hop count is kept in the reply's "redirects" property).
    QUrl redirectUrl(const QNetworkReply* reply) const {
        QUrl target = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (target.isEmpty()) return QUrl();
        target = reply->url().resolved(target);
        if (target == reply->url()) return QUrl();
        if (reply->property("redirects").toInt() >= 5) {
            qWarning() << "Network: too many redirects, giving up at" << reply->url().toString();
            return QUrl();
        }
        return target;
    }
};

#endif // DOWNLOADER_H
