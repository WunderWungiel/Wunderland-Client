// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPCATALOG_H
#define APPCATALOG_H

#include <QObject>
#include <QString>
#include <QUrl>
#include "applistmodel.h"

class StoreApi;
class UpdateTracker;

// The five app lists (latest, category, search, personal, updates), loaded and
// paginated from applist.php.
class AppCatalog : public QObject
{
    Q_OBJECT
    // The models are fixed objects; the *ListChanged signals say that a load finished.
    Q_PROPERTY(AppListModel *mainList READ mainList CONSTANT)
    Q_PROPERTY(AppListModel *categoryList READ categoryList CONSTANT)
    Q_PROPERTY(AppListModel *searchList READ searchList CONSTANT)
    Q_PROPERTY(AppListModel *personalList READ personalList CONSTANT)
    Q_PROPERTY(AppListModel *updateList READ updateList CONSTANT)
    // The term of the last loadSearchList (for the search field and the next page)
    Q_PROPERTY(QString lastSearchTerm READ lastSearchTerm NOTIFY lastSearchTermChanged)
    Q_ENUMS(ListId)

public:
    // listId values in loadingError()
    enum ListId { MainList, CategoryList, SearchList, PersonalList, UpdateList, ListCount };

    AppCatalog(StoreApi *api, UpdateTracker *updateTracker, QObject *parent = 0);

    AppListModel *mainList() const { return m_lists[MainList]; }
    AppListModel *categoryList() const { return m_lists[CategoryList]; }
    AppListModel *searchList() const { return m_lists[SearchList]; }
    AppListModel *personalList() const { return m_lists[PersonalList]; }
    AppListModel *updateList() const { return m_lists[UpdateList]; }
    QString lastSearchTerm() const { return m_lastSearchTerm; }

    Q_INVOKABLE void loadMainList();
    Q_INVOKABLE void loadCategoryList(int categoryId, int start, bool clean);
    Q_INVOKABLE void loadSearchList(QString searchTerm, int start, bool clean);
    Q_INVOKABLE void loadPersonalList(int start, bool clean);
    Q_INVOKABLE void loadUpdateList(int start, bool clean);
    // Only this app, into the update list (opening an app from a launch argument)
    Q_INVOKABLE void loadUpdateList(int id);
    // Repeats the last request of that list (after a loadingError)
    Q_INVOKABLE void retry(int listId);

public slots:
    // Marks the app as installed and up to date in every list
    void markInstalled(int id);

signals:
    // A load of that list finished (also when it added no rows)
    void mainListChanged();
    void searchListChanged();
    void categoryListChanged();
    void personalListChanged();
    void updateListChanged();
    void lastSearchTermChanged();
    // The server requires a newer Wunderland version
    void appListUpdate();
    // A list could not be loaded. listId: a ListId. error: a StoreApi::Error code.
    void loadingError(int listId, int error);

private slots:
    void listLoaded();

private:
    void loadList(const QUrl &url, int listId);
    void parseList(const QString &xml, int listId);

    StoreApi *m_api;
    UpdateTracker *m_updateTracker;
    QString m_wunderlandVersion;

    AppListModel *m_lists[ListCount];
    bool m_cleanList[ListCount]; // clear the list when the next load finishes
    QUrl m_lastUrl[ListCount];   // for retry()
    QString m_lastSearchTerm;
};

#endif // APPCATALOG_H
