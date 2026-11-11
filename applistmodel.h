#ifndef APPLISTMODEL_H
#define APPLISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QVariantMap>
#include "appentry.h"

// One app list (main, category, search, personal, updates) for QML. Each row is an AppEntry;
// the roles are AppRole with the names from appRoleNames().
class AppListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit AppListModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    int count() const { return entries.count(); }

    void append(const QList<AppEntry> &apps);
    void clear();

    // Sets the install state of every row with this app id (dataChanged per row).
    // Returns true if a row was found.
    bool updateInstalledState(int id, bool installed, bool updateAvailable);

    const AppEntry &entry(int row) const { return entries.at(row); }

    // All roles of a row by name, e.g. { "name": "Foo", "id": 12, ... }; empty if out of range.
    Q_INVOKABLE QVariantMap get(int row) const;

signals:
    void countChanged();

private:
    QList<AppEntry> entries;
};

#endif // APPLISTMODEL_H
