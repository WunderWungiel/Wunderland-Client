#include "news.h"
#include "applistparser.h"

// Roles of the news model
struct NewsEntry {
    enum NewsRoles {
        TitleRole = Qt::UserRole + 1,
        DescriptionRole = Qt::UserRole + 2,
        DateRole = Qt::UserRole + 3,
        LinkRole = Qt::UserRole + 4
    };
};

News::News(StoreApi *_api, QObject *parent) :
    QObject(parent)
{
    api = _api;

    QHash<int, QByteArray> roleNames;
    roleNames[NewsEntry::TitleRole] =  "title";
    roleNames[NewsEntry::DescriptionRole] = "description";
    roleNames[NewsEntry::DateRole] = "date";
    roleNames[NewsEntry::LinkRole] = "link";
    model = new RoleItemModel(roleNames);

    updating = false;

    url = api->url("/feed.xml").toString();
    htmlContent = "";
}

bool News::updateRunning() {
    return updating;
}

void News::fetch() {
    if (!updating) {
        qDebug() << "fetch feedUpdateStarted";
        qDebug() << "feedUrl: " + url;
        ApiReply *reply = api->get(QUrl(url));

        updating = true;
        connect(reply, SIGNAL(finished()),
                this, SLOT(feedFetchFinished()));
    }
}

void News::fetchHtml(QString link) {
    qDebug() << "fetch HTMLStarted " + link;
    ApiReply *reply = api->get(QUrl(link));
    connect(reply, SIGNAL(finished()),
            this, SLOT(htmlFetchFinished()));
}

QString News::getHtml() {
    return htmlContent;
}

void News::setFeedUrl(QString _url) {
    Q_UNUSED(_url);
}

QString News::feedUrl() {
    return url;
}

void News::htmlFetchFinished() {
    ApiReply *reply = qobject_cast<ApiReply *>(sender());
    if (!reply) return; // deletes itself

    QString html;
    if (reply->error() == StoreApi::NoError) html = reply->text();
    QTextDocument text;

    if (html != "") {
        //htmlContent = html.mid((html.indexOf("<content>")+9), (html.indexOf("</content>")-(html.indexOf("<content>")+9)));
        //text.setHtml(htmlContent);
        //htmlContent = text.toPlainText();
        //htmlContent = "<!DOCTYPE html>\n<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></head>\n<body style=\"background-color: #FFFFFF; color: #111111;\">"+htmlContent+"\n</body>\n</html>";
        htmlContent = html;
    } else {
        htmlContent = "<html><head></head><body style=\"background-color: #FFFFFF; color: #111111;\">Error while loading data.</body></html>";
    }

    emit htmlChanged();
}

void News::feedFetchFinished() {
    ApiReply *reply = qobject_cast<ApiReply *>(sender());
    if (!reply) return; // deletes itself

    if (reply->error() != StoreApi::NoError) {
        // Keep the old items, but stop the spinner
        updating = false;
        emit listChanged();
    } else {
        parseFeed(reply->text());
    }
}

void News::parseFeed(QString html) {
    QString error;
    QList<FeedItem> items = AppListParser::parseFeed(html, &error);
    if (items.isEmpty() && !error.isEmpty()) qWarning() << "News: bad feed:" << error;

    QTextDocument text;
    model->clear();

    for (int i = 0; i < items.count(); i++) {
        const FeedItem &item = items.at(i);

        // The description may be HTML: keep only its text
        QString description = item.description;
        description.remove("Continue reading");
        description.remove(QChar(0x2192)); // "&#8594;"
        text.setHtml(description);
        description = text.toPlainText().trimmed();

        QString link = item.link;
        //Mobilen Link, falls formula1.com
        if (link.startsWith("http://www.formula1.com/") || link.startsWith("http://formula1.com/")) {
            link.replace("http://www.formula1.com", "http://mobile.formula1.com");
            link.replace("http://formula1.com", "http://mobile.formula1.com");
            link.remove(".html");
        }

        QStandardItem* standardItem = new QStandardItem();
        standardItem->setData(item.title, NewsEntry::TitleRole);
        standardItem->setData(description, NewsEntry::DescriptionRole);
        standardItem->setData(item.date, NewsEntry::DateRole);
        standardItem->setData(link, NewsEntry::LinkRole);

        model->appendRow(standardItem);
    }

    emit listChanged();
    updating = false;
}
