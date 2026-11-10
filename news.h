#ifndef NEWS_H
#define NEWS_H

#include <QObject>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QTextDocument>

#include "roleitemmodel.h"

class News : public QObject
{
    Q_OBJECT

    Q_PROPERTY(RoleItemModel* _model READ _model NOTIFY listChanged)

private slots:
    void feedFetchFinished();
    void htmlFetchFinished();

public:
    explicit News(QObject *parent = 0);

    RoleItemModel* _model() {return model;}
    RoleItemModel *model;
    QNetworkAccessManager* comm_manager;

    QString url, htmlContent;

    bool updating;
    Q_INVOKABLE bool updateRunning () { return updating; }

    Q_INVOKABLE void fetch() {
        if (!updating) {
            qDebug() << "fetch feedUpdateStarted";
            qDebug() << "feedUrl: " + url;
            QNetworkReply *reply = comm_manager->get(QNetworkRequest(QUrl::fromPercentEncoding(url.toUtf8())));

            updating = true;
            connect(reply, SIGNAL(finished()),
                    this, SLOT(feedFetchFinished()));
        }
    }

    void parseFeed(QString html);

    Q_INVOKABLE void fetchHtml(QString link) {
        qDebug() << "fetch HTMLStarted " + link;
        QNetworkReply *reply = comm_manager->get(QNetworkRequest(QUrl(link)));
        connect(reply, SIGNAL(finished()),
                this, SLOT(htmlFetchFinished()));
    }
    Q_INVOKABLE QString getHtml() {
        return htmlContent;
    }

    Q_INVOKABLE void setFeedUrl (QString _url) {
    }

    Q_INVOKABLE QString feedUrl () {
        return url;
    }

    bool openInBrowser;
    Q_INVOKABLE bool getOpenInBrowser () { return openInBrowser; }
    Q_INVOKABLE void setOpenInBrowser (bool newSetting) {
        QSettings settings("WunderWungiel", "Wunderland");
        openInBrowser = newSetting;
        settings.setValue("news/openInBrowser", newSetting);
    }

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
    
signals:
    void listChanged();
    void htmlChanged();
};

#endif // NEWS_H
