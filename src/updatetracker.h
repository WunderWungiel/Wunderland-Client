// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPDATETRACKER_H
#define UPDATETRACKER_H

#include <QObject>
#include <QStringList>
#include "storeapi.h"
#include "appentry.h"

class AppManagement;

// Which store apps are installed, which have updates and the highest app id seen,
// plus the update check against the server. One instance, shared by the app
// lists, the install flow and the homescreen widget. It is the only owner of
// this persisted app state (apps/installedIds, apps/updateIds, apps/highestSeenId,
// settings/appsScanned, widget/lastUpdateTime); the keys are kept from older
// versions, so the state survives an upgrade.
class UpdateTracker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    UpdateTracker(StoreApi *api, AppManagement *appManagement, QObject *parent = 0);

    // Number of installed apps with an update
    int count() const { return m_updateIds.count(); }
    QStringList updateIds() const { return m_updateIds; }
    // Oldest install first
    QStringList installedIds() const { return m_installedIds; }

    // Asks the server for the installed apps and replaces the update list.
    // On an error the last known list is kept.
    void check();
    // True if check() has not succeeded in the last day (or resetLastCheck() was called)
    bool lastCheckOlderThanADay() const;
    // The next lastCheckOlderThanADay() returns true (the widget was deactivated or removed)
    void resetLastCheck();

    // For an app from a store list: sets app.installed (from the registry) and
    // app.updateAvailable, records its id as seen, and adds it to or removes it
    // from the installed list. Returns false if the app was in the installed list
    // but is not installed any more.
    bool track(AppEntry &app);

    // The installer for app id finished. The id goes to the end of the installed
    // list; on success it is also removed from the update list.
    void installFinished(int id, bool success);

signals:
    void countChanged();
    // After a successful check(). newApps: ids above the highest seen id.
    void checkFinished(int updates, int newApps);

private slots:
    void checkLoaded();

private:
    // First run: adds the installed apps known from apps.txt to the installed list
    void scanInstalledApps();
    void setUpdateIds(const QStringList &ids);
    void saveInstalledIds();

    StoreApi *m_api;
    AppManagement *m_appManagement;
    QStringList m_updateIds;
    QStringList m_installedIds;
    int m_highestSeenId;
};

#endif // UPDATETRACKER_H
