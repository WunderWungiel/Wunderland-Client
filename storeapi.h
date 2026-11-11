// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STOREAPI_H
#define STOREAPI_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QPair>
#include <QString>
#include <QTimer>
#include <QUrl>

class QIODevice;
class QNetworkAccessManager;
class QNetworkReply;
class Language;
class ApiReply;

typedef QList<QPair<QString, QString> > QueryItems;

// The one place that talks to the Wunderland Store server. Builds request
// URLs and runs GET requests (see ApiReply). Uses the shared
// QNetworkAccessManager owned by main.cpp.
class StoreApi : public QObject
{
    Q_OBJECT
    Q_ENUMS(Error)

public:
    // Error codes for ApiReply::error() and for the loadingError signals.
    // QML turns them into translated text.
    enum Error {
        NoError = 0,
        NetworkError,       // no connection, host not found, refused, ...
        TimeoutError,       // no data for a while
        HttpError,          // the server answered with status >= 400
        TooManyRedirects,
        InvalidResponse,    // set by callers: empty or unusable body
        WriteError,         // download(): writing to the output device failed
        Aborted             // ApiReply::abort() was called
    };

    // Common query parameters buildUrl() can add.
    enum QueryParam {
        DeviceParams  = 0x1, // os, resolution
        LanguageParam = 0x2, // language=EN,PL,... (left out when empty)
        AllParams     = DeviceParams | LanguageParam
    };

    StoreApi(QNetworkAccessManager *nam, Language *language, QObject *parent = 0);

    // "http://ovi.wunderwungiel.pl", or the "network/baseUrl" setting (for a
    // test server). No trailing slash.
    QString baseUrl() const;

    // baseUrl() + path, without a query. path starts with '/'.
    QUrl url(const QString &path) const;

    // url(path) with the given query items plus the common ones chosen by
    // params. Values are percent-encoded ('+', '&', non-ASCII); ',' is kept.
    QUrl buildUrl(const QString &path, const QueryItems &query = QueryItems(),
                  int params = AllParams) const;

    // Starts a GET request. Connect to ApiReply::finished() right away.
    ApiReply *get(const QUrl &url);

    // Like get(), but writes the body to out (open for writing) as it
    // arrives instead of keeping it in data(). Bodies of redirects and error
    // responses are not written. The caller owns out and closes it.
    ApiReply *download(const QUrl &url, QIODevice *out);

    QString os() const { return m_os; }
    QString resolution() const { return m_resolution; }

    QNetworkAccessManager *networkManager() const { return m_nam; }

private:
    QNetworkAccessManager *m_nam;
    Language *m_language;
    QString m_os;
    QString m_resolution;
};

// One logical request: follows redirects (up to a hop limit), gives up when
// no data arrives for a while, and checks the network error and HTTP status.
// Emits finished() exactly once and then deletes itself (deleteLater), so
// callers read it in their slot and must not keep the pointer (except to
// call abort() before finished()). Callers can attach context with
// setProperty("listId", ...).
class ApiReply : public QObject
{
    Q_OBJECT

public:
    // Stops the request and emits finished() with Aborted right away
    // (unless it already finished).
    void abort();

    StoreApi::Error error() const { return m_error; }
    QString errorString() const { return m_errorString; }
    int httpStatus() const { return m_httpStatus; }
    QUrl url() const { return m_url; }        // after redirects
    QByteArray data() const { return m_data; }
    QString text() const { return QString::fromUtf8(m_data.constData(), m_data.size()); }

signals:
    void finished();
    // Bytes of the current (final, after redirects) response. total is -1
    // when the server sends no Content-Length.
    void progress(qint64 received, qint64 total);

private slots:
    void onFinished();
    void onReadyRead();
    void onProgress(qint64 received, qint64 total);
    void onTimeout();

private:
    friend class StoreApi;
    ApiReply(QNetworkAccessManager *nam, QIODevice *out, QObject *parent);

    void start(const QUrl &url);
    bool isBodyWanted() const;
    bool writeOut(const QByteArray &data);
    void dropReply();
    void finish(StoreApi::Error error, const QString &errorString);

    QNetworkAccessManager *m_nam;
    QIODevice *m_out;
    QNetworkReply *m_reply;
    QTimer m_timer;
    QUrl m_url;
    int m_redirects;
    StoreApi::Error m_error;
    QString m_errorString;
    int m_httpStatus;
    QByteArray m_data;
    bool m_done;
};

#endif // STOREAPI_H
