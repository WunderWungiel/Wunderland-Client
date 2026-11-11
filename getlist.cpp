#include "getlist.h"
#include <QApplication>
#include <QStringList>

GetList::GetList(QObject *parent) :
    QObject(parent)
{
}

GetList::GetList(StoreApi *_api, Language *__language, QObject *parent) :
    QObject(parent)
{
    api = _api;
    _language = __language;

    wunderlandVersion = QApplication::instance()->applicationVersion();

    downloadProgress = 0;
    downloader = new Downloader(api, this);
    connect(downloader, SIGNAL(progressChanged(int)),
            this, SLOT(progressChangedReceived(int)));
    connect(downloader, SIGNAL(downloadDone()),
            this, SLOT(downloadDoneReceived()));
    connect(downloader, SIGNAL(downloadError(int)),
            this, SLOT(downloadErrorReceived(int)));
    // Connected once here; connecting per download made the handler run N times
    connect(&appManagement, SIGNAL(installServerFinished()),
            this, SLOT(installServerFinishedReceived()));

    updateManagement = new UpdateManagement(api, false, this);
    connect(updateManagement, SIGNAL(loadingFinished(QStringList)),
            this, SLOT(updateDataUpdated(QStringList)));
    updateManagement->loadList();

    QSettings settings("WunderWungiel", "Wunderland");
    updateIds = settings.value("apps/updateIds", updateIds).toStringList();

    for (int i=0; i<5; i++) list[i] = new AppListModel(this);

    currentList = 0;
    cleanList[0] = false;
    cleanList[1] = false;
    cleanList[2] = false;
    cleanList[3] = false;
    cleanList[4] = false;

    lastSearchTerm = "";
}

static QPair<QString, QString> queryItem(const char *key, const QString &value) {
    return qMakePair(QString::fromLatin1(key), value);
}

void GetList::loadMainList() {
    QueryItems query;
    query << queryItem("latest", "true") << queryItem("count", "7");
    cleanList[0] = true;
    loadList(api->buildUrl("/applist.php", query), 0);
}

void GetList::loadCategoryList(int categoryId, int start, bool clean) {
    QueryItems query;
    query << queryItem("category", QString::number(categoryId))
          << queryItem("start", QString::number(start));
    cleanList[1] = clean;
    loadList(api->buildUrl("/applist.php", query), 1);
}

void GetList::loadSearchList(QString searchTerm, int start, bool clean) {
    QueryItems query;
    query << queryItem("search", searchTerm)
          << queryItem("start", QString::number(start));
    cleanList[2] = clean;
    lastSearchTerm = searchTerm;
    loadList(api->buildUrl("/applist.php", query), 2);
}

void GetList::loadPersonalList(int start, bool clean) {
    QSettings settings("WunderWungiel", "Wunderland");
    QStringList installedIds = settings.value("apps/installedIds").toStringList();
    QStringList ids;
    for (int i=(installedIds.length()-1); i>=0; i--) ids << installedIds.at(i); // newest first

    QueryItems query;
    query << queryItem("id", ids.join(",")) << queryItem("start", QString::number(start));
    cleanList[3] = clean;
    loadList(api->buildUrl("/applist.php", query, StoreApi::DeviceParams), 3);
}

void GetList::loadUpdateList(int start, bool clean) {
    QueryItems query;
    query << queryItem("id", updateIds.join(",")) << queryItem("start", QString::number(start));
    cleanList[4] = clean;
    loadList(api->buildUrl("/applist.php", query, StoreApi::DeviceParams), 4);
}

void GetList::loadUpdateList(int id) {
    QueryItems query;
    query << queryItem("id", QString::number(id)) << queryItem("start", "0");
    cleanList[4] = true;
    loadList(api->buildUrl("/applist.php", query, StoreApi::DeviceParams), 4);
}

void GetList::loadList(QUrl url, int listId) {
    ApiReply *reply = api->get(url);
    reply->setProperty("listId", listId);
    connect(reply, SIGNAL(finished()),
            this, SLOT(listLoaded()));
}

void GetList::listLoaded() {
    ApiReply *reply = qobject_cast<ApiReply *>(sender());
    if (!reply) return; // deletes itself

    int listId = reply->property("listId").toInt();
    if (reply->error() != StoreApi::NoError) {
        emit loadingError(listId, reply->error());
        return;
    }
    parseList(reply->text(), listId);
}

void GetList::parseList(QString html, int listId) {
    AppListResult result = AppListParser::parse(html);
    if (!result.ok) qWarning() << "GetList: bad XML for list" << listId << ":" << result.errorString;

    // A broken document still shows the apps read before the error
    if (result.ok || !result.apps.isEmpty()) {
        QString minVersion = result.minVersion;

        if (!appManagement.compareVersions(wunderlandVersion, minVersion)) {
            QSettings settings("WunderWungiel", "Wunderland");
            int highestSeenId = settings.value("apps/highestSeenId", 0).toInt();
            QStringList installedIds = settings.value("apps/installedIds").toStringList();
            QList<AppEntry> apps;

            if (cleanList[listId]) {
                list[listId]->clear();
                cleanList[listId] = false;
            }

            for (int a = 0; a < result.apps.count(); a++) {
                AppEntry app = result.apps.at(a);
                int id = app.id;
                // ID hasn't been seen, set as highest seen id
                if (id > highestSeenId) highestSeenId = id;

                QString installedVersion = appManagement.getVersion(app.uid);
                app.installed = !installedVersion.isEmpty();

                // If installed add to installedIds-List
                if (app.installed && !installedIds.contains(QString::number(id))) {
                    // If not in installedId list check for update, otherwise it will be checked within updatelist
                    if (AppManagement::isUpdate(installedVersion, app.version)) {
                        app.updateAvailable = true;
                        updateIds.append(QString::number(id));
                        emit updateCountChanged();
                    }

                    installedIds.append(QString::number(id));
                } else if (updateIds.contains(QString::number(id))) {
                    app.updateAvailable = true;
                }

                // Is part of installed list but not installed (anymore) remove
                if (!app.installed && installedIds.contains(QString::number(id))) {
                    installedIds.removeAll(QString::number(id));
                    // Add to list if not in personal list
                    if (listId != 3) apps.append(app);
                } else {
                    apps.append(app);
                }
            }
            list[listId]->append(apps);

            //Save seen Ids
            settings.setValue("apps/highestSeenId", highestSeenId);
            settings.setValue("apps/installedIds", installedIds);

            switch (listId) {
            case 0: emit mainListChanged(); break;
            case 1: emit categoryListChanged(); break;
            case 2: emit searchListChanged(); break;
            case 3: emit personalListChanged(); break;
            case 4: emit updateListChanged(); break;
            }
        } else {
            // A newer client version is required
            qDebug() << "newer version required";
            emit appListUpdate();
        }
    } else {
        // Empty or not XML
        emit loadingError(listId, StoreApi::InvalidResponse);
    }
}

int GetList::buttonAction(int index) {
    /*  onButtonPressed:
     *  0 - download
     *  1 - update
     *  4 - launch app
     *  6 - currently downloading item
     */
    currentItem = list[currentList]->get(index);

    if (downloadingItem == currentItem) return 6;

    if (!currentItem.value("installed").toBool()) return 0;

    // Read the registry again: the app may have been removed since the list was loaded
    QString installedVersion = appManagement.getVersion(currentItem.value("uid").toString());
    if (installedVersion.isEmpty()) return 0;
    if (AppManagement::isUpdate(installedVersion, currentItem.value("version").toString())) return 1;
    return 4;
}

bool GetList::buttonPressed(int actionId) {

    // Reset downloadProgress
    downloadProgress = 0;
    emit progressChanged();

    if (actionId == 0 || actionId == 1) {
        // Download/update
        // Set first: a failed start emits downloadError right away, which clears it
        downloadingItem = currentItem;
        downloader->startDownload(currentItem.value("download").toString(), currentItem.value("downloadsize").toLongLong(), QString::number(currentItem.value("id").toInt()), _language->currentLanguage());
        return isDownloading();
    } else if (actionId == 4) {
        // Launch app
        appManagement.launchApp(currentItem.value("uid").toString());
    }

    // return if download progress shall be displayed
    return true;
}
