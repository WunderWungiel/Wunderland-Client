#include "appcatalog.h"
#include "storeapi.h"
#include "updatetracker.h"
#include "applistparser.h"
#include "versioncompare.h"
#include <QApplication>
#include <QStringList>
#include <QDebug>

AppCatalog::AppCatalog(StoreApi *api, UpdateTracker *updateTracker, QObject *parent) :
    QObject(parent), m_api(api), m_updateTracker(updateTracker)
{
    m_wunderlandVersion = QApplication::instance()->applicationVersion();

    for (int i = 0; i < ListCount; i++) {
        m_lists[i] = new AppListModel(this);
        m_cleanList[i] = false;
    }
}

static QPair<QString, QString> queryItem(const char *key, const QString &value) {
    return qMakePair(QString::fromLatin1(key), value);
}

void AppCatalog::loadMainList() {
    QueryItems query;
    query << queryItem("latest", "true") << queryItem("count", "7");
    m_cleanList[MainList] = true;
    loadList(m_api->buildUrl("/applist.php", query), MainList);
}

void AppCatalog::loadCategoryList(int categoryId, int start, bool clean) {
    QueryItems query;
    query << queryItem("category", QString::number(categoryId))
          << queryItem("start", QString::number(start));
    m_cleanList[CategoryList] = clean;
    loadList(m_api->buildUrl("/applist.php", query), CategoryList);
}

void AppCatalog::loadSearchList(QString searchTerm, int start, bool clean) {
    QueryItems query;
    query << queryItem("search", searchTerm)
          << queryItem("start", QString::number(start));
    m_cleanList[SearchList] = clean;
    m_lastSearchTerm = searchTerm;
    loadList(m_api->buildUrl("/applist.php", query), SearchList);
}

void AppCatalog::loadPersonalList(int start, bool clean) {
    QStringList installedIds = m_updateTracker->installedIds();
    QStringList ids;
    for (int i = installedIds.count() - 1; i >= 0; i--) ids << installedIds.at(i); // newest first

    QueryItems query;
    query << queryItem("id", ids.join(",")) << queryItem("start", QString::number(start));
    m_cleanList[PersonalList] = clean;
    loadList(m_api->buildUrl("/applist.php", query, StoreApi::DeviceParams), PersonalList);
}

void AppCatalog::loadUpdateList(int start, bool clean) {
    QueryItems query;
    query << queryItem("id", m_updateTracker->updateIds().join(","))
          << queryItem("start", QString::number(start));
    m_cleanList[UpdateList] = clean;
    loadList(m_api->buildUrl("/applist.php", query, StoreApi::DeviceParams), UpdateList);
}

void AppCatalog::loadUpdateList(int id) {
    QueryItems query;
    query << queryItem("id", QString::number(id)) << queryItem("start", "0");
    m_cleanList[UpdateList] = true;
    loadList(m_api->buildUrl("/applist.php", query, StoreApi::DeviceParams), UpdateList);
}

void AppCatalog::markInstalled(int id) {
    for (int i = 0; i < ListCount; i++) m_lists[i]->updateInstalledState(id, true, false);
}

void AppCatalog::loadList(const QUrl &url, int listId) {
    ApiReply *reply = m_api->get(url);
    reply->setProperty("listId", listId);
    connect(reply, SIGNAL(finished()),
            this, SLOT(listLoaded()));
}

void AppCatalog::listLoaded() {
    ApiReply *reply = qobject_cast<ApiReply *>(sender());
    if (!reply) return; // deletes itself

    int listId = reply->property("listId").toInt();
    if (reply->error() != StoreApi::NoError) {
        emit loadingError(listId, reply->error());
        return;
    }
    parseList(reply->text(), listId);
}

void AppCatalog::parseList(const QString &xml, int listId) {
    AppListResult result = AppListParser::parse(xml);
    if (!result.ok) qWarning() << "AppCatalog: bad XML for list" << listId << ":" << result.errorString;

    // A broken document still shows the apps read before the error
    if (!result.ok && result.apps.isEmpty()) {
        // Empty or not XML
        emit loadingError(listId, StoreApi::InvalidResponse);
        return;
    }

    if (isNewer(m_wunderlandVersion, result.minVersion)) {
        qDebug() << "newer version required";
        emit appListUpdate();
        return;
    }

    if (m_cleanList[listId]) {
        m_lists[listId]->clear();
        m_cleanList[listId] = false;
    }

    QList<AppEntry> apps;
    for (int a = 0; a < result.apps.count(); a++) {
        AppEntry app = result.apps.at(a);
        // An app that was removed since is left out of the personal list
        if (m_updateTracker->track(app) || listId != PersonalList) apps.append(app);
    }
    m_lists[listId]->append(apps);

    switch (listId) {
    case MainList: emit mainListChanged(); break;
    case CategoryList: emit categoryListChanged(); break;
    case SearchList: emit searchListChanged(); break;
    case PersonalList: emit personalListChanged(); break;
    case UpdateList: emit updateListChanged(); break;
    }
}
