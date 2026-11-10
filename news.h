#ifndef NEWS_H
#define NEWS_H

#include <QObject>
#include <QSettings>
#include <QDebug>
#include <QTextDocument>

#include "roleitemmodel.h"
#include "storeapi.h"

class News : public QObject
{
    Q_OBJECT

    Q_PROPERTY(RoleItemModel* _model READ _model NOTIFY listChanged)

private slots:
    void feedFetchFinished();
    void htmlFetchFinished();

public:
    explicit News(StoreApi *api, QObject *parent = 0);

    RoleItemModel* _model() {return model;}
    RoleItemModel *model;
    StoreApi *api;

    QString url, htmlContent;

    bool updating;
    Q_INVOKABLE bool updateRunning () { return updating; }

    Q_INVOKABLE void fetch() {
        if (!updating) {
            qDebug() << "fetch feedUpdateStarted";
            qDebug() << "feedUrl: " + url;
            ApiReply *reply = api->get(QUrl(url));

            updating = true;
            connect(reply, SIGNAL(finished()),
                    this, SLOT(feedFetchFinished()));
        }
    }

    void parseFeed(QString html);

    Q_INVOKABLE void fetchHtml(QString link) {
        qDebug() << "fetch HTMLStarted " + link;
        ApiReply *reply = api->get(QUrl(link));
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
    
signals:
    void listChanged();
    void htmlChanged();
};

#endif // NEWS_H
