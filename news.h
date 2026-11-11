#ifndef NEWS_H
#define NEWS_H

#include <QObject>
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
    Q_INVOKABLE bool updateRunning();

    Q_INVOKABLE void fetch();
    void parseFeed(QString html);

    Q_INVOKABLE void fetchHtml(QString link);
    Q_INVOKABLE QString getHtml();

    Q_INVOKABLE void setFeedUrl(QString _url);
    Q_INVOKABLE QString feedUrl();

signals:
    void listChanged();
    void htmlChanged();
};

#endif // NEWS_H
