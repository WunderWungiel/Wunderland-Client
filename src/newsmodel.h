// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NEWSMODEL_H
#define NEWSMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "applistparser.h"

// The news feed for QML. Each row is a FeedItem; the roles are title, description
// (plain text), date and link.
class NewsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    // The numbers are the ones the RoleItemModel used before
    enum NewsRole {
        TitleRole = Qt::UserRole + 1,
        DescriptionRole,
        DateRole,
        LinkRole
    };

    explicit NewsModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    int count() const { return items.count(); }

    // Replaces all rows
    void setItems(const QList<FeedItem> &newItems);

signals:
    void countChanged();

private:
    QList<FeedItem> items;
};

#endif // NEWSMODEL_H
