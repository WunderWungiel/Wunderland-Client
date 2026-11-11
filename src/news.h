// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

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
    // A feed fetch is running
    Q_PROPERTY(bool updating READ updating NOTIFY updatingChanged)
    // The page fetchHtml() loaded last
    Q_PROPERTY(QString html READ html NOTIFY htmlChanged)

public:
    explicit News(StoreApi *api, QObject *parent = 0);

    NewsModel *model() const { return m_model; }

    bool updating() const { return m_updating; }
    QString html() const { return m_html; }

    // Loads the feed and replaces the rows; ignored while a fetch is running
    Q_INVOKABLE void fetch();

    // Loads a news page; htmlChanged() when it is done, then html has it
    Q_INVOKABLE void fetchHtml(const QString &link);

signals:
    void listChanged();
    void updatingChanged();
    void htmlChanged();

private slots:
    void feedFetchFinished();
    void htmlFetchFinished();

private:
    void parseFeed(const QString &xml);

    StoreApi *api;
    NewsModel *m_model;
    QString m_html;
    QPointer<ApiReply> htmlReply;   // the page that was asked for last
    bool m_updating;
};

#endif // NEWS_H
