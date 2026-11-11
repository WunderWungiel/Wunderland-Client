#include "applistmodel.h"

AppListModel::AppListModel(QObject *parent) :
    QAbstractListModel(parent)
{
    setRoleNames(appRoleNames());
}

int AppListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return entries.count();
}

QVariant AppListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= entries.count()) return QVariant();
    return entries.at(index.row()).data(role);
}

void AppListModel::append(const QList<AppEntry> &apps)
{
    if (apps.isEmpty()) return;
    beginInsertRows(QModelIndex(), entries.count(), entries.count() + apps.count() - 1);
    entries.append(apps);
    endInsertRows();
    emit countChanged();
}

void AppListModel::clear()
{
    if (entries.isEmpty()) return;
    beginResetModel();
    entries.clear();
    endResetModel();
    emit countChanged();
}

bool AppListModel::updateInstalledState(int id, const QString &installed, bool updateAvailable)
{
    bool found = false;
    for (int row = 0; row < entries.count(); row++) {
        AppEntry &e = entries[row];
        if (e.id != id) continue;
        found = true;
        e.installed = installed;
        e.updateAvailable = updateAvailable;
        QModelIndex idx = index(row);
        emit dataChanged(idx, idx);
    }
    return found;
}

QVariantMap AppListModel::get(int row) const
{
    QVariantMap res;
    if (row < 0 || row >= entries.count()) return res;

    const AppEntry &e = entries.at(row);
    QHash<int, QByteArray> names = roleNames();
    QHashIterator<int, QByteArray> i(names);
    while (i.hasNext()) {
        i.next();
        res[QString::fromLatin1(i.value())] = e.data(i.key());
    }
    return res;
}
