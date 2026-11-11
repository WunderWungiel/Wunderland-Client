#ifndef UPDATETRACKER_H
#define UPDATETRACKER_H

#include <QObject>
#include <QStringList>
#include "storeapi.h"
#include "appentry.h"

class AppManagement;

// Which store apps are installed, which have updates and the highest app id seen
// (persisted as apps/installedIds, apps/updateIds, apps/highestSeenId), plus the
// update check against the server. One instance, shared by the app lists, the
// install flow and the homescreen widget.
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
    // True if check() has not succeeded in the last day (or the widget reset it)
    bool lastCheckOlderThanADay() const;

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
    void setUpdateIds(const QStringList &ids);
    void saveInstalledIds();

    StoreApi *m_api;
    AppManagement *m_appManagement;
    QStringList m_updateIds;
    QStringList m_installedIds;
    int m_highestSeenId;
};

#endif // UPDATETRACKER_H
