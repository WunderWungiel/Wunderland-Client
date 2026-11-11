#include "newsmodel.h"

NewsModel::NewsModel(QObject *parent) :
    QAbstractListModel(parent)
{
    QHash<int, QByteArray> names;
    names[TitleRole] = "title";
    names[DescriptionRole] = "description";
    names[DateRole] = "date";
    names[LinkRole] = "link";
    setRoleNames(names);
}

int NewsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return items.count();
}

QVariant NewsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= items.count()) return QVariant();
    const FeedItem &item = items.at(index.row());
    switch (role) {
    case TitleRole: return item.title;
    case DescriptionRole: return item.description;
    case DateRole: return item.date;
    case LinkRole: return item.link;
    default: return QVariant();
    }
}

void NewsModel::setItems(const QList<FeedItem> &newItems)
{
    bool countChanges = newItems.count() != items.count();
    beginResetModel();
    items = newItems;
    endResetModel();
    if (countChanges) emit countChanged();
}
