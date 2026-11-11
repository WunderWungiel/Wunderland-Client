#ifndef GETLIST_H
#define GETLIST_H

#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include "roleitemmodel.h"
#include "appmanagement.h"
#include "updatemanagement.h"
#include "language.h"
#include "downloader.h"
#include "storeapi.h"
#include "applistparser.h"

class GetList : public QObject
{
    Q_PROPERTY(RoleItemModel *_mainList READ _mainList NOTIFY mainListChanged)
    Q_PROPERTY(RoleItemModel *_categoryList READ _categoryList NOTIFY categoryListChanged)
    Q_PROPERTY(RoleItemModel *_searchList READ _searchList NOTIFY searchListChanged)
    Q_PROPERTY(RoleItemModel *_personalList READ _personalList NOTIFY personalListChanged)
    Q_PROPERTY(RoleItemModel *_updateList READ _updateList NOTIFY updateListChanged)
    Q_PROPERTY(int _updates READ _updates NOTIFY updateCountChanged)
    Q_PROPERTY(int _downloadProgress READ _downloadProgress NOTIFY progressChanged)
    Q_OBJECT

public:
    explicit GetList(QObject *parent = 0);
    GetList(StoreApi *api, Language *__language, QObject *parent = 0);
    
    Q_INVOKABLE void loadMainList();
    Q_INVOKABLE void loadCategoryList(int categoryId, int start, bool clean);
    Q_INVOKABLE void loadSearchList(QString searchTerm, int start, bool clean);
    Q_INVOKABLE QString getLastSearchTerm() { return lastSearchTerm; }
    Q_INVOKABLE void loadPersonalList(int start, bool clean);
    Q_INVOKABLE void loadUpdateList(int start, bool clean);
    Q_INVOKABLE void loadUpdateList(int id);

    void loadList(QUrl url, int listId);
    void parseList(QString html, int listId);

    Q_INVOKABLE void setCurrentList(int i) { currentList = i; }

    Q_INVOKABLE int buttonAction(int index);
    /* ActionId to do when downloadButton pressed
       0/1 = Download/update, 4 = Launch */
    Q_INVOKABLE bool buttonPressed(int actionId);

    int _downloadProgress() { return downloadProgress; }
    Q_INVOKABLE bool isDownloading() {
        if (downloadingItem == emptyItem) return false;
        else return true;
    }
    Q_INVOKABLE bool cancelDownload() {
        if (downloader->cancelDownload()) {
            downloadingItem = emptyItem;
            return true;
        } else {
            return false;
        }
    }

private:
    StoreApi *api;
    Language *_language;

    RoleItemModel *list[5];
    RoleItemModel* _mainList() { return list[0]; }
    RoleItemModel* _categoryList() { return list[1]; }
    RoleItemModel* _searchList() { return list[2]; }
    QString lastSearchTerm;
    RoleItemModel* _personalList() { return list[3]; }
    RoleItemModel* _updateList() { return list[4]; }

    int currentList;
    int listLoading;
    bool cleanList[5];

    AppManagement appManagement;

    UpdateManagement* updateManagement;
    QStringList updateIds;
    int _updates() { return updateIds.count(); }

    Downloader *downloader;
    int downloadProgress;

    QString wunderlandVersion;

    QVariantMap currentItem, downloadingItem, emptyItem;

    //QString debug;

signals:
    void mainListChanged();
    void searchListChanged();
    void categoryListChanged();
    void personalListChanged();
    void updateListChanged();
    //To display that a new AppList version is required
    void appListUpdate();
    // A list could not be loaded. listId: 0 main, 1 category, 2 search,
    // 3 personal, 4 updates. error: a StoreApi::Error code.
    void loadingError(int listId, int error);

    void updateCountChanged();

    void downloadDone();
    void downloadError(int code);
    void installServerFinished(int statusCode);

    void progressChanged();
public slots:
    void listLoaded();
    void progressChangedReceived(int percent) {
        downloadProgress = percent;
        emit progressChanged();
    }
    void downloadDoneReceived() {
        qDebug() << "downloadDone";

        appManagement.startInstallServerListening();

        emit downloadDone();
    }
    void downloadErrorReceived(int code) {
        downloadingItem = emptyItem;
        emit downloadError(code);
    }
    void updateDataUpdated(QStringList _updateIds) {
        updateIds = _updateIds;
        emit updateCountChanged();
    }

    void installServerFinishedReceived() {
        // Delete file after installation
        downloader->deleteFile();

        // Installation was successful if the downloaded version is now installed
        QString uid = downloadingItem.value("uid").toString();
        QString installedVersion = appManagement.getVersion(uid);
        if (installedVersion.length() > 0 && QString::compare(installedVersion, downloadingItem.value("version").toString()) == 0) {
            // Go through all 5 lists
            for (int i=0; i<5; i++) {
                // Find updated/installed item in all lists and update "installed" field
                for (int l=0; l<list[i]->rowCount(); l++) {
                    if (list[i]->getModelData(l).value("id").toString() == downloadingItem.value("id").toString()) {
                        QStandardItem *tempItem = list[i]->item(l);

                        tempItem->setData(uid+"1", Qt::UserRole + 13);
                        // Set update value to false
                        tempItem->setData("false", Qt::UserRole + 35);

                        // Update item
                        list[i]->setItem(l, tempItem);

                        // Update currentItem
                        if (currentItem == downloadingItem) currentItem = list[i]->getModelData(l);

                        // Notify list changes
                        switch (i) {
                            case 0: mainListChanged(); break;
                            case 1: categoryListChanged(); break;
                            case 2: searchListChanged(); break;
                            case 3: personalListChanged(); break;
                            case 4: updateListChanged(); break;
                        }

                        break;
                    }
                }
            }

            // Update order in installed IDs list
            QSettings settings("WunderWungiel", "Wunderland");
            QStringList installedIds = settings.value("apps/installedIds").toStringList();
            if (installedIds.contains(QString::number(downloadingItem.value("id").toInt()))) installedIds.removeAll(QString::number(downloadingItem.value("id").toInt()));
            installedIds.append(QString::number(downloadingItem.value("id").toInt()));
            settings.setValue("apps/installedIds", installedIds);

            // Remove item from updateIds list
            if (updateIds.contains(downloadingItem.value("id").toString())) {
                updateIds.removeAll(downloadingItem.value("id").toString());
                settings.setValue("apps/updateIds", updateIds);
                emit updateCountChanged();
            }

            downloadingItem = emptyItem;

            emit installServerFinished(0);
        } else {
            // Update order in installed IDs list
            QSettings settings("WunderWungiel", "Wunderland");
            QStringList installedIds = settings.value("apps/installedIds").toStringList();
            if (installedIds.contains(QString::number(downloadingItem.value("id").toInt()))) installedIds.removeAll(QString::number(downloadingItem.value("id").toInt()));
            installedIds.append(QString::number(downloadingItem.value("id").toInt()));
            settings.setValue("apps/installedIds", installedIds);

            downloadingItem = emptyItem;

            // Installation failed, return Error
            emit installServerFinished(1);
        }
    }
};

#endif // GETLIST_H
