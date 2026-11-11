#ifndef APPMANAGEMENT_H
#define APPMANAGEMENT_H

#include <QObject>
#include <QString>
#include <QStringList>

// Installed apps, for the update check and the detail page. The native calls
// are in platform/apps.*.
class AppManagement : public QObject
{
    Q_OBJECT
public:
    explicit AppManagement(QObject *parent = 0);
    
    // Installed version ("1.2.3"), empty if not installed
    QString getVersion(QString uid);

    // True if the app with this UID is installed in a version older than availableVersion.
    // Callers that already have the installed version (getVersion(), empty = not installed)
    // use isNewer() from versioncompare.h, so the registry isn't read twice.
    bool hasUpdate(QString uid, QString availableVersion);

    bool launchApp(QString uid);
    Q_INVOKABLE bool checkAppInstalled(QString uid);

    // Store IDs of the installed apps that are listed in the bundled apps.txt
    // (for the first run, before the store lists have been opened). False if
    // the installed apps could not be read.
    bool installedStoreIds(QStringList &ids);
};

#endif // APPMANAGEMENT_H
