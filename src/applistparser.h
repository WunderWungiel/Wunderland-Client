// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLISTPARSER_H
#define APPLISTPARSER_H

#include <QString>
#include <QList>
#include <QHash>
#include "appentry.h"

struct AppListResult
{
    QString minVersion;
    QList<AppEntry> apps;    // the complete <app> elements read before any error
    bool ok;                 // the whole document was well-formed
    QString errorString;
};

struct FeedItem
{
    QString title;
    QString description;     // as sent (HTML possible); entities and CDATA decoded
    QString date;            // <pubDate>, unchanged
    QString link;
};

// Parses the XML the store server sends, with QXmlStreamReader. Fields can be in any
// order, missing fields are empty, entities and CDATA are decoded.
class AppListParser
{
public:
    // applist.php: <client|applist><minversion/><app>...</app>...</client>
    static AppListResult parse(const QString &xml);

    // A flat document such as version.xml/changelog.xml: <xml><message/><url/></xml>.
    // Returns the text of every element below the root, by tag name.
    static QHash<QString, QString> parseFields(const QString &xml, QString *errorString = 0);

    // feed.xml (RSS 2.0): the <item>s of the channel.
    static QList<FeedItem> parseFeed(const QString &xml, QString *errorString = 0);
};

#endif // APPLISTPARSER_H
