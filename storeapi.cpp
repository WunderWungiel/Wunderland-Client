// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#include "storeapi.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>

#include "language.h"
#include "platform/device.h"

static const char kDefaultBaseUrl[] = "http://ovi.wunderwungiel.pl";
// Give up when nothing arrives for this long (Qt 4 QNAM has no timeout).
static const int kTimeoutMs = 30000;
static const int kMaxRedirects = 5;

static void addQueryItem(QUrl &url, const QString &key, const QString &value)
{
    // Qt 4's addQueryItem() leaves '+' unencoded (servers read it as a
    // space), so encode ourselves. ',' separates id/language lists.
    url.addEncodedQueryItem(QUrl::toPercentEncoding(key),
                            QUrl::toPercentEncoding(value, ","));
}

// ---------------------------------------------------------------- StoreApi

StoreApi::StoreApi(QNetworkAccessManager *nam, Language *language, QObject *parent)
    : QObject(parent),
      m_nam(nam),
      m_language(language)
{
    m_os = Platform::osVersion();

    // "1" for 640x480 screens (E6), in either orientation
    const QRect screen = QApplication::desktop()->screenGeometry();
    m_resolution = (screen.width() == 480 || screen.height() == 480)
            ? QLatin1String("1") : QLatin1String("0");

    qDebug() << "StoreApi:" << baseUrl() << "os" << m_os << "resolution" << m_resolution;
}

QString StoreApi::baseUrl() const
{
    QSettings settings("WunderWungiel", "Wunderland");
    QString base = settings.value("network/baseUrl").toString().trimmed();
    if (base.isEmpty())
        base = QLatin1String(kDefaultBaseUrl);
    while (base.endsWith(QLatin1Char('/')))
        base.chop(1);
    return base;
}

QUrl StoreApi::url(const QString &path) const
{
    return QUrl(baseUrl() + path);
}

QUrl StoreApi::buildUrl(const QString &path, const QueryItems &query, int params) const
{
    QUrl u = url(path);
    for (int i = 0; i < query.size(); ++i)
        addQueryItem(u, query.at(i).first, query.at(i).second);

    if ((params & LanguageParam) && m_language) {
        const QString language = m_language->languagesForUrl();
        if (!language.isEmpty())
            addQueryItem(u, "language", language);
    }
    if (params & DeviceParams) {
        addQueryItem(u, "os", m_os);
        addQueryItem(u, "resolution", m_resolution);
    }
    return u;
}

ApiReply *StoreApi::get(const QUrl &url)
{
    ApiReply *reply = new ApiReply(m_nam, 0, this);
    reply->start(url);
    return reply;
}

ApiReply *StoreApi::download(const QUrl &url, QIODevice *out)
{
    ApiReply *reply = new ApiReply(m_nam, out, this);
    reply->start(url);
    return reply;
}

// ---------------------------------------------------------------- ApiReply

ApiReply::ApiReply(QNetworkAccessManager *nam, QIODevice *out, QObject *parent)
    : QObject(parent),
      m_nam(nam),
      m_out(out),
      m_reply(0),
      m_redirects(0),
      m_error(StoreApi::NoError),
      m_httpStatus(0),
      m_done(false)
{
    m_timer.setSingleShot(true);
    m_timer.setInterval(kTimeoutMs);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

void ApiReply::start(const QUrl &url)
{
    m_url = url;
    qDebug() << "StoreApi: GET" << url.toString();
    m_reply = m_nam->get(QNetworkRequest(url));
    connect(m_reply, SIGNAL(finished()), this, SLOT(onFinished()));
    connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onProgress(qint64,qint64)));
    if (m_out)
        connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    m_timer.start();
}

void ApiReply::abort()
{
    if (m_done)
        return;
    qDebug() << "StoreApi: aborted" << m_url.toString();
    dropReply();
    finish(StoreApi::Aborted, QLatin1String("Aborted"));
}

// False while the current response is a redirect or an error page, whose
// body must not end up in the output device.
bool ApiReply::isBodyWanted() const
{
    if (!m_reply)
        return false;
    if (!m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl().isEmpty())
        return false;
    return m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() < 400;
}

bool ApiReply::writeOut(const QByteArray &data)
{
    if (data.isEmpty())
        return true;
    if (m_out->write(data) == data.size())
        return true;
    qWarning() << "StoreApi: cannot write the download:" << m_out->errorString();
    dropReply();
    finish(StoreApi::WriteError, m_out->errorString());
    return false;
}

void ApiReply::onReadyRead()
{
    if (!m_reply || sender() != m_reply)
        return;
    const QByteArray data = m_reply->readAll(); // read either way, or it piles up
    if (isBodyWanted())
        writeOut(data);
}

void ApiReply::onProgress(qint64 received, qint64 total)
{
    if (!m_reply || sender() != m_reply)
        return;
    m_timer.start(); // data arrived, restart the stall timer
    if (isBodyWanted())
        emit progress(received, total);
}

void ApiReply::onTimeout()
{
    if (!m_reply)
        return;
    qWarning() << "StoreApi: no answer for" << kTimeoutMs / 1000 << "s, giving up:" << m_url.toString();
    dropReply();
    finish(StoreApi::TimeoutError, QLatin1String("Timed out"));
}

// Forgets the request in flight without its finished() reaching us.
void ApiReply::dropReply()
{
    QNetworkReply *reply = m_reply;
    m_reply = 0;
    if (!reply)
        return;
    disconnect(reply, 0, this, 0);
    // Aborting a reply that already failed makes Qt 4.8 complain
    // ("Internal problem, this method must only be called once").
    if (reply->error() == QNetworkReply::NoError && !reply->isFinished())
        reply->abort();
    reply->deleteLater();
}

void ApiReply::onFinished()
{
    QNetworkReply *reply = m_reply;
    if (!reply || sender() != reply)
        return;
    m_reply = 0;
    m_timer.stop();
    reply->deleteLater();

    m_httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    QUrl target = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!target.isEmpty()) {
        target = reply->url().resolved(target);
        if (target == reply->url() || m_redirects >= kMaxRedirects) {
            qWarning() << "StoreApi: too many redirects, giving up at" << reply->url().toString();
            finish(StoreApi::TooManyRedirects, QLatin1String("Too many redirects"));
            return;
        }
        ++m_redirects;
        qDebug() << "StoreApi: redirected to" << target.toString();
        start(target);
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        const StoreApi::Error error = m_httpStatus >= 400 ? StoreApi::HttpError : StoreApi::NetworkError;
        qWarning() << "StoreApi: request failed:" << m_url.toString()
                   << "status" << m_httpStatus << "error" << int(reply->error()) << reply->errorString();
        finish(error, reply->errorString());
        return;
    }
    if (m_httpStatus >= 400) {
        qWarning() << "StoreApi: HTTP" << m_httpStatus << "for" << m_url.toString();
        finish(StoreApi::HttpError, QString("HTTP %1").arg(m_httpStatus));
        return;
    }

    if (m_out) {
        if (!writeOut(reply->readAll()))
            return;
    } else {
        m_data = reply->readAll();
    }
    finish(StoreApi::NoError, QString());
}

void ApiReply::finish(StoreApi::Error error, const QString &errorString)
{
    if (m_done)
        return;
    m_done = true;
    m_timer.stop();
    m_error = error;
    m_errorString = errorString;
    emit finished();
    deleteLater();
}
