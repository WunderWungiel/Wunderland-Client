#include "news.h"

#include <QDebug>
#include <QTextDocument>

#include "applistparser.h"

News::News(StoreApi *_api, QObject *parent) :
    QObject(parent),
    api(_api),
    model(new NewsModel(this)),
    updating(false)
{
}

void News::fetch() {
    if (updating) return;

    QUrl url = api->url("/feed.xml");
    qDebug() << "News: fetching" << url.toString();
    ApiReply *reply = api->get(url);
    updating = true;
    connect(reply, SIGNAL(finished()), this, SLOT(feedFetchFinished()));
}

void News::fetchHtml(const QString &link) {
    qDebug() << "News: fetching page" << link;
    // A reply for a page asked for earlier is ignored (see htmlFetchFinished)
    htmlReply = api->get(QUrl(link));
    connect(htmlReply, SIGNAL(finished()), this, SLOT(htmlFetchFinished()));
}

void News::htmlFetchFinished() {
    ApiReply *reply = qobject_cast<ApiReply *>(sender());
    if (!reply || reply != htmlReply) return; // deletes itself

    QString html;
    if (reply->error() == StoreApi::NoError) html = reply->text();

    if (!html.isEmpty()) {
        htmlContent = html;
    } else {
        htmlContent = "<html><head></head><body style=\"background-color: #FFFFFF; color: #111111;\">Error while loading data.</body></html>";
    }

    emit htmlChanged();
}

void News::feedFetchFinished() {
    ApiReply *reply = qobject_cast<ApiReply *>(sender());
    if (!reply) return; // deletes itself

    // On an error the old items are kept
    if (reply->error() == StoreApi::NoError) parseFeed(reply->text());

    updating = false;
    emit listChanged();
}

void News::parseFeed(const QString &xml) {
    QString error;
    QList<FeedItem> items = AppListParser::parseFeed(xml, &error);
    if (items.isEmpty() && !error.isEmpty()) qWarning() << "News: bad feed:" << error;

    QTextDocument text;
    for (int i = 0; i < items.count(); i++) {
        // The description may be HTML: keep only its text
        QString description = items.at(i).description;
        description.remove("Continue reading");
        description.remove(QChar(0x2192)); // "&#8594;"
        text.setHtml(description);
        items[i].description = text.toPlainText().trimmed();
    }

    model->setItems(items);
}
