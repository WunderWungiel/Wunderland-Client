// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#include "downloader.h"

#include <QDebug>
#include <QDir>
#include <QUrl>

#include "platform/apps.h"
#include "platform/device.h"

Downloader::Downloader(StoreApi *api, QObject *parent) :
    QObject(parent),
    m_api(api),
    m_installerThreadId(0)
{
}

void Downloader::startDownload(const QString &source, qint64 size, const QString &id, const QString &language) {
    if (m_reply) return;

    // Fire and forget: a failed statistics request does not stop the download
    QueryItems query;
    query << qMakePair(QString("id"), id) << qMakePair(QString("language"), language);
    ApiReply *stats = m_api->get(m_api->buildUrl("/applist-download.php", query, StoreApi::DeviceParams));
    connect(stats, SIGNAL(finished()), this, SLOT(onStatsFinished()));

    QString url = source;
    if (!url.startsWith("http")) url.prepend(m_api->baseUrl() + "/static/content/files/");

    // 10% headroom, as before
    start(url, size + size / 10);
}

void Downloader::startDownloadWithoutStats(const QString &source) {
    if (m_reply) return;
    start(source, 0);
}

bool Downloader::start(const QString &source, qint64 minFree) {
    const QString folder = Platform::downloadFolder(minFree);
    if (folder.isEmpty()) {
        qWarning() << "Downloader: no drive with" << minFree << "bytes free";
        emit downloadError(NoSpaceError);
        return false;
    }

    QString fileName = QUrl(source).path();
    fileName = fileName.mid(fileName.lastIndexOf('/') + 1);
    if (fileName.isEmpty()) fileName = "download.sis";

    m_file.setFileName(folder + "/" + fileName);
    if (!m_file.open(QIODevice::WriteOnly)) {
        qWarning() << "Downloader: cannot write" << m_file.fileName() << m_file.errorString();
        emit downloadError(WriteError);
        return false;
    }

    qDebug() << "Downloader:" << source << "to" << m_file.fileName();
    m_reply = m_api->download(QUrl(source), &m_file);
    connect(m_reply, SIGNAL(progress(qint64,qint64)),
            this, SLOT(onProgress(qint64,qint64)));
    connect(m_reply, SIGNAL(finished()),
            this, SLOT(onFinished()));
    return true;
}

void Downloader::onProgress(qint64 received, qint64 total) {
    // total is -1 (or 0) when the server sends no Content-Length
    if (total <= 0) return;
    emit progressChanged((int)(((double)received / total) * 100));
}

void Downloader::onFinished() {
    ApiReply *reply = qobject_cast<ApiReply *>(sender());
    if (!reply || reply != m_reply) return;
    m_reply = 0; // deletes itself

    StoreApi::Error error = reply->error();
    // QFile buffers; a full disk can show up only here
    if (error == StoreApi::NoError && !m_file.flush()) {
        qWarning() << "Downloader: cannot write" << m_file.fileName() << m_file.errorString();
        error = StoreApi::WriteError;
    }
    m_file.close();

    if (error == StoreApi::NoError) {
        qDebug() << "Downloader: done" << m_file.fileName() << m_file.size() << "bytes";
        if (!Platform::openInstaller(m_file.fileName(), &m_installerThreadId)) {
            deleteFile();
            emit downloadError(InstallerError);
            return;
        }
        emit downloadDone();
        return;
    }

    // Don't leave an error page or a partial file behind
    deleteFile();
    if (error == StoreApi::Aborted) return;

    qWarning() << "Downloader: download failed:" << reply->url().toString() << reply->errorString();
    emit downloadError(error == StoreApi::WriteError ? WriteError : ConnectionError);
}

void Downloader::onStatsFinished() {
    ApiReply *reply = qobject_cast<ApiReply *>(sender());
    if (!reply) return;
    if (reply->error() != StoreApi::NoError || reply->text().trimmed() != "0")
        qWarning() << "Downloader: statistics request failed:" << reply->errorString() << reply->text().left(100);
}

bool Downloader::cancelDownload() {
    if (!m_reply) return false;
    // Emits finished() right away, which deletes the partial file
    m_reply->abort();
    return true;
}

bool Downloader::deleteFile() {
    if (m_file.fileName().isEmpty()) return false;
    return m_file.remove();
}
