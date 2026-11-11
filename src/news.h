#ifndef NEWS_H
#define NEWS_H

#include <QObject>
#include <QPointer>

#include "newsmodel.h"
#include "storeapi.h"

// The store news feed (feed.xml) and the pages it links to.
class News : public QObject
{
    Q_OBJECT

    Q_PROPERTY(NewsModel *model READ model CONSTANT)

public:
    explicit News(StoreApi *api, QObject *parent = 0);

    NewsModel *model() const { return m_model; }

    Q_INVOKABLE bool updateRunning() const { return updating; }
    // Loads the feed and replaces the rows; ignored while a fetch is running
    Q_INVOKABLE void fetch();

    // Loads a news page; htmlChanged() when it is done, then getHtml() has it
    Q_INVOKABLE void fetchHtml(const QString &link);
    Q_INVOKABLE QString getHtml() const { return htmlContent; }

signals:
    void listChanged();
    void htmlChanged();

private slots:
    void feedFetchFinished();
    void htmlFetchFinished();

private:
    void parseFeed(const QString &xml);

    StoreApi *api;
    NewsModel *m_model;
    QString htmlContent;
    QPointer<ApiReply> htmlReply;   // the page that was asked for last
    bool updating;
};

#endif // NEWS_H
