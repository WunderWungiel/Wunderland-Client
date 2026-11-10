#include "getlist.h"
#include <QApplication>
#include <QDesktopServices>
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
    connect(&downloader, SIGNAL(progressChanged(int)),
            this, SLOT(progressChangedReceived(int)));
    connect(&downloader, SIGNAL(downloadDone()),
            this, SLOT(downloadDoneReceived()));
    connect(&downloader, SIGNAL(downloadError(int)),
            this, SLOT(downloadErrorReceived(int)));
    // Connected once here; connecting per download made the handler run N times
    connect(&appManagement, SIGNAL(installServerFinished()),
            this, SLOT(installServerFinishedReceived()));

    downloadFromStoreStarted = false;
    activateCount = 0;

    updateManagement = new UpdateManagement(api, false, this);
    connect(updateManagement, SIGNAL(loadingFinished(QStringList)),
            this, SLOT(updateDataUpdated(QStringList)));
    updateManagement->loadList();

    QSettings settings("WunderWungiel", "Wunderland");
    updateIds = settings.value("apps/updateIds", updateIds).toStringList();

    useUnsigned = settings.value("apps/unsigned", false).toBool();

    struct ListEntry {
        enum ListRoles {
            IdRole = Qt::UserRole + 1,
            NameRole = Qt::UserRole + 2,
            UidRole = Qt::UserRole + 3,
            UidStoreRole = Qt::UserRole + 4,
            UidUnsignedRole = Qt::UserRole + 5,
            IconRole = Qt::UserRole + 6,
            VersionRole = Qt::UserRole + 7,
            VersionStoreRole = Qt::UserRole + 8,
            VersionUnsignedRole = Qt::UserRole + 9,
            VersionDateRole = Qt::UserRole + 10,
            VersionDateStoreRole = Qt::UserRole + 11,
            VersionDateUnsignedRole = Qt::UserRole + 12,
            InstalledUidRole = Qt::UserRole + 13,
            CategoryRole = Qt::UserRole + 14,
            LanguageRole = Qt::UserRole + 15,
            OsRole = Qt::UserRole + 16,
            DeveloperRole = Qt::UserRole + 17,
            MailRole = Qt::UserRole + 18,
            WebsiteRole = Qt::UserRole + 19,
            TwitterRole = Qt::UserRole + 20,
            FacebookRole = Qt::UserRole + 21,
            DonationRole = Qt::UserRole + 22,
            AASRole = Qt::UserRole + 23,
            PriceRole = Qt::UserRole + 24,
            DescriptionRole = Qt::UserRole + 25,
            Image1Role = Qt::UserRole + 26,
            Image2Role = Qt::UserRole + 27,
            Image3Role = Qt::UserRole + 28,
            TagsRole = Qt::UserRole + 29,
            ChangelogRole = Qt::UserRole + 30,
            UnsignedNoteRole = Qt::UserRole + 31,
            DownloadRole = Qt::UserRole + 32,
            DownloadStoreRole = Qt::UserRole + 33,
            DownloadUnsignedRole = Qt::UserRole + 34,
            UpdateRole = Qt::UserRole + 35,
            Image4Role = Qt::UserRole + 36,
            Image5Role = Qt::UserRole + 37,
            DownloadSizeRole = Qt::UserRole + 38,
            DownloadUnsignedSizeRole = Qt::UserRole + 39
        };
    };
    QHash<int, QByteArray> roleNames;
    roleNames[ListEntry::IdRole] = "id";
    roleNames[ListEntry::NameRole] =  "name";
    roleNames[ListEntry::UidRole] = "uid";
    roleNames[ListEntry::UidStoreRole] = "uidstore";
    roleNames[ListEntry::UidUnsignedRole] = "uidunsigned";
    roleNames[ListEntry::IconRole] = "icon";
    roleNames[ListEntry::VersionRole] = "version";
    roleNames[ListEntry::VersionStoreRole] = "versionstore";
    roleNames[ListEntry::VersionUnsignedRole] = "versionunsigned";
    roleNames[ListEntry::VersionDateRole] = "versiondate";
    roleNames[ListEntry::VersionDateStoreRole] = "versiondatestore";
    roleNames[ListEntry::VersionDateUnsignedRole] = "versiondateunsigned";
    roleNames[ListEntry::InstalledUidRole] = "installed";
    roleNames[ListEntry::CategoryRole] = "category";
    roleNames[ListEntry::LanguageRole] = "language";
    roleNames[ListEntry::OsRole] = "os";
    roleNames[ListEntry::DeveloperRole] = "developer";
    roleNames[ListEntry::MailRole] = "mail";
    roleNames[ListEntry::WebsiteRole] = "website";
    roleNames[ListEntry::TwitterRole] = "twitter";
    roleNames[ListEntry::FacebookRole] = "facebook";
    roleNames[ListEntry::DonationRole] = "donation";
    roleNames[ListEntry::AASRole] = "aas";
    roleNames[ListEntry::PriceRole] = "price";
    roleNames[ListEntry::DescriptionRole] = "description";
    roleNames[ListEntry::Image1Role] = "image1";
    roleNames[ListEntry::Image2Role] = "image2";
    roleNames[ListEntry::Image3Role] = "image3";
    roleNames[ListEntry::TagsRole] = "tags";
    roleNames[ListEntry::ChangelogRole] = "changelog";
    roleNames[ListEntry::UnsignedNoteRole] = "unsignednote";
    roleNames[ListEntry::DownloadRole] = "download";
    roleNames[ListEntry::DownloadStoreRole] = "downloadstore";
    roleNames[ListEntry::DownloadUnsignedRole] = "downloadunsigned";
    roleNames[ListEntry::UpdateRole] = "updateAvailable";
    roleNames[ListEntry::Image4Role] = "image4";
    roleNames[ListEntry::Image5Role] = "image5";
    roleNames[ListEntry::DownloadSizeRole] = "downloadsize";
    roleNames[ListEntry::DownloadUnsignedSizeRole] = "downloadunsignedsize";

    //RoleItemModel *_list[2] = {new RoleItemModel(roleNames), new RoleItemModel(roleNames), new RoleItemModel(roleNames)};
    //list = _list;
    list[0] = new RoleItemModel(roleNames);
    list[1] = new RoleItemModel(roleNames);
    list[2] = new RoleItemModel(roleNames);
    list[3] = new RoleItemModel(roleNames);
    list[4] = new RoleItemModel(roleNames);

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
    loadList(api->buildUrl("/applist.php", query, StoreApi::DeviceParams | StoreApi::UnsignedParam), 3);
}

void GetList::loadUpdateList(int start, bool clean) {
    QueryItems query;
    query << queryItem("id", updateIds.join(",")) << queryItem("start", QString::number(start));
    cleanList[4] = clean;
    loadList(api->buildUrl("/applist.php", query, StoreApi::DeviceParams | StoreApi::UnsignedParam), 4);
}

void GetList::loadUpdateList(int id) {
    QueryItems query;
    query << queryItem("id", QString::number(id)) << queryItem("start", "0");
    cleanList[4] = true;
    loadList(api->buildUrl("/applist.php", query, StoreApi::DeviceParams | StoreApi::UnsignedParam), 4);
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

            if (cleanList[listId]) {
                list[listId]->clear();
                cleanList[listId] = false;
            }

            for (int a = 0; a < result.apps.count(); a++) {
                const AppFields &app = result.apps.at(a);
                QStandardItem* item = new QStandardItem();

                int id = app.value("id").toInt();
                item->setData(id, Qt::UserRole + 1);
                // ID hasn't been seen, set as highest seen id
                if (id > highestSeenId) highestSeenId = id;

                item->setData(app.value("name"), Qt::UserRole + 2);

                QString uid = app.value("uid");
                item->setData(uid, Qt::UserRole + 3);

                QString uidstore = app.value("uidstore");
                item->setData(uidstore, Qt::UserRole + 4);

                QString uidunsigned = app.value("uidunsigned");
                item->setData(uidunsigned, Qt::UserRole + 5);

                item->setData(app.value("icon"), Qt::UserRole + 6);

                QString version = app.value("version");
                item->setData(version, Qt::UserRole + 7);
                QString versionstore = app.value("versionstore");
                item->setData(versionstore, Qt::UserRole + 8);
                QString versionunsigned = app.value("versionunsigned");
                item->setData(versionunsigned, Qt::UserRole + 9);

                QDateTime versiondate = QDateTime::fromString(app.value("versiondate"), "yyyy-MM-dd hh:mm:ss");
                item->setData(versiondate, Qt::UserRole + 10);
                QDateTime versiondatestore = QDateTime::fromString(app.value("versiondatestore"), "yyyy-MM-dd hh:mm:ss");
                item->setData(versiondatestore, Qt::UserRole + 11);
                QDateTime versiondateunsigned = QDateTime::fromString(app.value("versiondateunsigned"), "yyyy-MM-dd hh:mm:ss");
                item->setData(versiondateunsigned, Qt::UserRole + 12);

                // Save installed UID
                QString installedVersion = appManagement.getVersion(uid);

                if (installedVersion.length() > 0 && uid.length() > 0 && QString::compare(uid, uidunsigned) == 0) {
                    /* uid and uidunsigned are the same. The unsigned version must then be the newer one,
                      and the installed version is either <= self-signed or >= unsigned
                      */
                    if (appManagement.compareVersions(version, installedVersion)) {
                        // Installed version is newer than the last self-signed one, so the unsigned one is installed
                        item->setData(uidunsigned+"2", Qt::UserRole + 13);
                    } else {
                        item->setData(uid+"1", Qt::UserRole + 13);
                    }
                }
                else if (installedVersion.length() > 0) item->setData(uid+"1", Qt::UserRole + 13);
                else {
                    installedVersion = appManagement.getVersion(uidunsigned);

                    if (installedVersion.length() > 0) item->setData(uidunsigned+"2", Qt::UserRole + 13);
                    else {
                        installedVersion = appManagement.getVersion(uidstore);
                        if (installedVersion.length() > 0) item->setData(uidstore+"3", Qt::UserRole + 13);
                        else item->setData("0", Qt::UserRole + 13);
                    }
                }

                // If installed add to installedIds-List
                if (installedVersion.length() > 0 && !installedIds.contains(QString::number(id))) {
                    // If not in installedId list check for update, otherwise it will be checked within updatelist
                    if (updateManagement->checkForUpdate(uid, uidstore, (useUnsigned ? uidunsigned : ""), version, versionstore, (useUnsigned ? versionunsigned : ""))) {
                        item->setData("true", Qt::UserRole + 35);
                        updateIds.append(QString::number(id));
                        emit updateCountChanged();
                    } else {
                        item->setData("false", Qt::UserRole + 35);
                    }

                    installedIds.append(QString::number(id));
                } else if (updateIds.contains(QString::number(id))) {
                    item->setData("true", Qt::UserRole + 35);
                } else {
                    item->setData("false", Qt::UserRole + 35);
                }

                item->setData(app.value("category").toInt(), Qt::UserRole + 14);
                item->setData(app.value("language"), Qt::UserRole + 15);
                item->setData(app.value("os"), Qt::UserRole + 16);
                item->setData(app.value("developer"), Qt::UserRole + 17);
                item->setData(app.value("mail"), Qt::UserRole + 18);
                item->setData(app.value("website"), Qt::UserRole + 19);
                item->setData(app.value("twitter"), Qt::UserRole + 20);
                item->setData(app.value("facebook"), Qt::UserRole + 21);
                item->setData(app.value("donation"), Qt::UserRole + 22);
                item->setData(app.value("aas"), Qt::UserRole + 23);
                item->setData(app.value("price").toDouble(), Qt::UserRole + 24);
                item->setData(app.value("description"), Qt::UserRole + 25);
                item->setData(app.value("image1"), Qt::UserRole + 26);
                item->setData(app.value("image2"), Qt::UserRole + 27);
                item->setData(app.value("image3"), Qt::UserRole + 28);
                item->setData(app.value("image4"), Qt::UserRole + 36);
                item->setData(app.value("image5"), Qt::UserRole + 37);
                item->setData(app.value("tags"), Qt::UserRole + 29);
                item->setData(app.value("changelog"), Qt::UserRole + 30);
                item->setData(app.value("unsignednote"), Qt::UserRole + 31);
                item->setData(app.value("download"), Qt::UserRole + 32);
                item->setData(app.value("downloadsize"), Qt::UserRole + 38);
                item->setData(app.value("downloadstore"), Qt::UserRole + 33);
                item->setData(app.value("downloadunsigned"), Qt::UserRole + 34);
                item->setData(app.value("downloadunsignedsize"), Qt::UserRole + 39);


                // Is part of installed list but not installed (anymore) remove
                if (installedVersion.length() == 0 && installedIds.contains(QString::number(id))) {
                    installedIds.removeAll(QString::number(id));
                    // Add to list if not in personal list
                    if (listId != 3) list[listId]->appendRow(item);
                } else {
                    list[listId]->appendRow(item);
                }
            }

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
     *  0 - direct download (only one version available)
     *  1 - direct selfsigned update
     *  2 - direct unsigned update
     *  3 - direct Store update
     *  4 - launch app
     *  5 - update multiple options (open SelectVersion)
     *  6 - currently downloading item
     *  7 - download multiple options (open SelectVersion)
     */
    int onButtonPressed = -1;

    // Download/LaunchButton
    currentItem = list[currentList]->getModelData(index);

    // If this item is the downloadingItem, set 6
    if (downloadingItem == currentItem) {
        onButtonPressed = 6;
    } else {
        //Get installed UID
        QString uid = currentItem.value("installed").toString();
        //Get source of installation (0=not installed, 1=selfsigned, 2=unsigned, 3=store)
        int source = uid.mid(uid.length()-1).toInt();

        uid.chop(1);

        QString installedVersion = appManagement.getVersion(uid);

        if (source==0) {
            // Not installed, download
            onButtonPressed = 0;

            if (currentItem.value("download").toString() != "") onButtonPressed++;
            if (currentItem.value("downloadunsigned").toString() != "" && useUnsigned) onButtonPressed++;
            if (currentItem.value("downloadstore").toString() != "" && onButtonPressed == 0) onButtonPressed++;
            if (onButtonPressed > 1) onButtonPressed = 7;
            else onButtonPressed = 0;
        } else {
            int updateCount = 0;
            if (source == 1) {
                if (appManagement.compareVersions(installedVersion, currentItem.value("version").toString())) updateCount++;
                if (useUnsigned) if (appManagement.compareVersions(installedVersion, currentItem.value("versionunsigned").toString())) updateCount++;
            } else if (source == 2) {
                if (appManagement.compareVersions(installedVersion, currentItem.value("versionunsigned").toString())) updateCount++;
            } else if (source == 3) {
                if (appManagement.compareVersions(installedVersion, currentItem.value("version").toString())) updateCount++;
                if (appManagement.compareVersions(installedVersion, currentItem.value("versionunsigned").toString())) updateCount++;
                if (appManagement.compareVersions(installedVersion, currentItem.value("versionstore").toString())) updateCount++;
            }

            if (updateCount == 0) {
                // No update, launch
                onButtonPressed = 4;
            } else if (updateCount == 1) {
                // Direct updates, doesn't require SelectVersion to be displayed
                // source: self-signed, no update for unsigned available => 1 update self-signed to self-signed
                if (source == 1 && (!useUnsigned || !appManagement.compareVersions(installedVersion, currentItem.value("versionunsigned").toString()))) {
                    onButtonPressed = 1;
                } else if (source == 2) {
                    onButtonPressed = 2;
                } else if (source == 3 && !appManagement.compareVersions(installedVersion, currentItem.value("version").toString()) && (!useUnsigned || !appManagement.compareVersions(installedVersion, currentItem.value("versionunsigned").toString()))) {
                    onButtonPressed = 3;
                }
            }

            if (onButtonPressed == -1) {
                // Update selection required (QML -> open SelectVersion)
                onButtonPressed = 5;
                /*if (appManagement.compareVersions(installedVersion, currentItem.value("version").toString())) {
                    // Self-Signed is newer than installed, offer update if unsigned is not installed to selfsigned
                    if (source!=3 || useUnsigned) onButtonPressed = 1;
                    // else start unsigned
                    else onButtonPressed = 4;
                } else if (appManagement.compareVersions(installedVersion, currentItem.value("versionunsigned").toString())) {
                    // Unsigned version is newer than installed, offer update from unsigned or Store to unsigned
                    if (source>1) onButtonPressed = 2;
                    // else start Self-Signed
                    else onButtonPressed = 4;
                } else if (appManagement.compareVersions(installedVersion, currentItem.value("versionstore").toString())) {
                    // Store version is newer than installed, offer update from Store versions to Store
                    if (source == 2) onButtonPressed = 3;
                    // else start other versions
                    else onButtonPressed = 4;
                } else {
                    // No update available, but app installed => launch
                    onButtonPressed = 4;
                }*/
            }
        }
    }

    return onButtonPressed;
}

bool GetList::buttonPressed(int actionId) {

    // Reset downloadProgress
    downloadProgress = 0;
    emit progressChanged();

    QSettings settings("WunderWungiel", "Wunderland");

    // Manipulate actionId, if 0; direct download
    if (actionId == 0) {
        if (currentItem.value("download").toString() != "") actionId = 1;
        else if (currentItem.value("downloadunsigned").toString() != "" && useUnsigned) actionId = 2;
        else if (currentItem.value("downloadstore").toString() != "") actionId = 3;
    }

    if (actionId == 1) {
        // Download/update self-signed
        downloader.startDownload(currentItem.value("download").toString(), currentItem.value("downloadsize").toLongLong(), QString::number(currentItem.value("id").toInt()), _language->currentLanguage(), api->os(), api->resolution());
        downloadingItem = currentItem;
        //debug += "1: " + QString::number(downloadingItem.value("id").toInt());
    } else if (actionId == 2) {
        // Download/update unsigned
        downloader.startDownload(currentItem.value("downloadunsigned").toString(), currentItem.value("downloadunsignedsize").toLongLong(), QString::number(currentItem.value("id").toInt()), _language->currentLanguage(), api->os(), api->resolution());
        downloadingItem = currentItem;
        //debug += "2: " + QString::number(downloadingItem.value("id").toInt());
    } else if (actionId == 3) {
        // Download/update Store version (open Store)
        downloadingItem = currentItem;
        downloadFromStoreStarted = true;
        activateCount = 0;
        //openInDefaultBrowser(currentItem.value("downloadstore").toString());
        QDesktopServices::openUrl(QUrl(currentItem.value("downloadstore").toString()));
        //debug += "3: " + QString::number(downloadingItem.value("id").toInt());
    } else if (actionId == 4) {
        // Launch app
        QString installedUID = currentItem.value("installed").toString();
        installedUID.chop(1);
        /*bool*/ appManagement.launchApp(installedUID);
    }

    // return if download progress shall be displayed
    if (actionId == 3) return false;
    else return true;
}
