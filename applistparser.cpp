#include "applistparser.h"

#include <QXmlStreamReader>

namespace {

QString errorText(const QXmlStreamReader &reader)
{
    return QString("%1 (line %2, column %3)")
            .arg(reader.errorString())
            .arg(reader.lineNumber())
            .arg(reader.columnNumber());
}

// Reads the children of the current element into a map, up to its end tag.
QHash<QString, QString> readChildren(QXmlStreamReader &reader)
{
    QHash<QString, QString> fields;
    while (reader.readNextStartElement()) {
        const QString name = reader.name().toString();
        fields.insert(name, reader.readElementText(QXmlStreamReader::IncludeChildElements));
    }
    return fields;
}

} // namespace

AppListResult AppListParser::parse(const QString &xml)
{
    AppListResult result;
    QXmlStreamReader reader(xml);

    // The root is <client> on the current server and <applist> in old versions;
    // accept any root and look for <minversion> and <app> anywhere below it.
    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isStartElement()) continue;

        if (reader.name() == "minversion") {
            result.minVersion = reader.readElementText();
        } else if (reader.name() == "app") {
            QHash<QString, QString> fields = readChildren(reader);
            if (!reader.hasError()) result.apps.append(AppEntry::fromFields(fields));
        }
    }

    result.ok = !reader.hasError();
    if (!result.ok) result.errorString = errorText(reader);
    return result;
}

QHash<QString, QString> AppListParser::parseFields(const QString &xml, QString *errorString)
{
    QHash<QString, QString> fields;
    QXmlStreamReader reader(xml);

    if (reader.readNextStartElement()) fields = readChildren(reader);

    if (errorString) *errorString = reader.hasError() ? errorText(reader) : QString();
    return fields;
}

QList<FeedItem> AppListParser::parseFeed(const QString &xml, QString *errorString)
{
    // The server's feed.xml currently has whitespace before the XML declaration and
    // "<rssversion=...>" instead of "<rss version=...>", which no XML parser accepts.
    // Start at <channel> instead; the stray </rss> at the end is then an error after
    // the last item, which is fine.
    QString data = xml;
    const int channel = data.indexOf("<channel");
    if (channel > 0) data = data.mid(channel);

    QList<FeedItem> items;
    QXmlStreamReader reader(data);
    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isStartElement() || reader.name() != "item") continue;

        QHash<QString, QString> fields = readChildren(reader);
        if (reader.hasError()) break;

        FeedItem item;
        item.title = fields.value("title").trimmed();
        item.description = fields.value("description").trimmed();
        item.date = fields.value("pubDate").trimmed();
        item.link = fields.value("link").trimmed();
        items.append(item);
    }

    if (errorString) *errorString = reader.hasError() ? errorText(reader) : QString();
    return items;
}
