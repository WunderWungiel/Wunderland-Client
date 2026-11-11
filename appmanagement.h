#ifndef APPMANAGEMENT_H
#define APPMANAGEMENT_H

#include <QObject>
#include <QString>

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

    bool appsScanned();
    // Maps the installed apps to store IDs (apps.txt) on the first run
    void scanApps();
};

#endif // APPMANAGEMENT_H
