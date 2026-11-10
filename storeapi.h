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
        InvalidResponse     // set by callers: empty or unusable body
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
// callers read it in their slot and must not keep the pointer. Callers can
// attach context with setProperty("listId", ...).
class ApiReply : public QObject
{
    Q_OBJECT

public:
    StoreApi::Error error() const { return m_error; }
    QString errorString() const { return m_errorString; }
    int httpStatus() const { return m_httpStatus; }
    QUrl url() const { return m_url; }        // after redirects
    QByteArray data() const { return m_data; }
    QString text() const { return QString::fromUtf8(m_data.constData(), m_data.size()); }

signals:
    void finished();

private slots:
    void onFinished();
    void onProgress();
    void onTimeout();

private:
    friend class StoreApi;
    ApiReply(QNetworkAccessManager *nam, QObject *parent);

    void start(const QUrl &url);
    void dropReply();
    void finish(StoreApi::Error error, const QString &errorString);

    QNetworkAccessManager *m_nam;
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
