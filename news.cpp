#include "news.h"

News::News(StoreApi *_api, QObject *parent) :
    QObject(parent)
{
    api = _api;

    struct NewsEntry {
        enum NewsRoles {
            TitleRole = Qt::UserRole + 1,
            DescriptionRole = Qt::UserRole + 2,
            DateRole = Qt::UserRole + 3,
            LinkRole = Qt::UserRole + 4
        };
    };
    QHash<int, QByteArray> roleNames;
    roleNames[NewsEntry::TitleRole] =  "title";
    roleNames[NewsEntry::DescriptionRole] = "description";
    roleNames[NewsEntry::DateRole] = "date";
    roleNames[NewsEntry::LinkRole] = "link";
    model = new RoleItemModel(roleNames);

    updating = false;

    QSettings settings("WunderWungiel", "Wunderland");
    openInBrowser = settings.value("news/openInBrowser", false).toBool();

    url = api->url("/feed.xml").toString();
    htmlContent = "";
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
    QTextDocument text;

    int lastPos = 0; int lastPosItem = 0;

    model->clear();

    while (html.indexOf("<item>", lastPosItem) != -1) {
        lastPosItem = html.indexOf("<item>", lastPosItem) + 6;
        QString item = html.mid(lastPosItem, ((html.indexOf("</item>", lastPosItem))-lastPosItem));
        lastPos = item.indexOf("<title>") + 7;

        //Title
        QString title = item.mid(lastPos, (item.indexOf("</title>", lastPos) - lastPos));
        title.replace("&quot;", "\"");

        text.setHtml(title);
        title = text.toPlainText();


        //Description
        lastPos = item.indexOf("<description>") + 13;
        QString description = item.mid(lastPos, (item.indexOf("</description>", lastPos) - lastPos));

        if (description.indexOf("<![CDATA[") != -1) {
            description.remove("<![CDATA[");
            description.remove("]]>");
        }
        while (description.mid(0,1) == " ") description.remove(0,1);

        while (description.indexOf("<") != -1) {
            int klammerPos = description.indexOf("<");
            int klammerLength = description.indexOf(">", klammerPos) - klammerPos + 1;
            description.remove(klammerPos, klammerLength);
        }
        while (description.indexOf("&lt;") != -1) {
            int klammerPos = description.indexOf("&lt;");
            int klammerLength = description.indexOf("&gt;", klammerPos) - klammerPos + 4;
            description.remove(klammerPos, klammerLength);
        }
        description.remove("Continue reading");
        description.remove("&#8594;");

        text.setHtml(description);
        description = text.toPlainText();


        lastPos = item.indexOf("<pubDate>") + 9;
        //Publication time of the news item
        QString date = item.mid(lastPos, item.indexOf("</pubDate>", lastPos)-lastPos);
        //Convert to local time
        //date = timeChanges.tweetZeitUmrechnen(date);


        //Link
        lastPos = item.indexOf("<link>") + 6;
        QString link = item.mid(lastPos, item.indexOf("</link>", lastPos)-lastPos);

        //Mobilen Link, falls formula1.com
        if (link.startsWith("http://www.formula1.com/") || link.startsWith("http://formula1.com/")) {
            link.replace("http://www.formula1.com", "http://mobile.formula1.com");
            link.replace("http://formula1.com", "http://mobile.formula1.com");
            link.remove(".html");
        }


        /*qDebug() << title;
        qDebug() << description;
        qDebug() << date;
        qDebug() << link;*/

        //tweet in Model speichern
        QStandardItem* standardItem = new QStandardItem();
        standardItem->setData(title, Qt::UserRole+1);
        standardItem->setData(description, Qt::UserRole+2);
        standardItem->setData(date, Qt::UserRole+3);
        standardItem->setData(link, Qt::UserRole+4);

        model->appendRow(standardItem);
    }

    emit listChanged();
    updating = false;
}
