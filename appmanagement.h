#ifndef APPMANAGEMENT_H
#define APPMANAGEMENT_H

#include <QObject>
#include <sisregistrysession.h>
#include <sisregistryentry.h>
#include <QDebug>
#include <QSettings>

#include <QStringList>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include <apgcli.h> // for RApaLsSession
#include <apacmdln.h> // for CApaCommandLine

class AppManagement : public QObject
{
    Q_OBJECT
public:
    explicit AppManagement(QObject *parent = 0);
    
    QString getVersion(QString uid);

    // True if the app with this UID is installed in a version older than availableVersion.
    // Callers that already have the installed version (getVersion(), empty = not installed)
    // use isNewer() from versioncompare.h, so the registry isn't read twice.
    bool hasUpdate(QString uid, QString availableVersion);

    bool launchApp(QString uid);
    Q_INVOKABLE bool checkAppInstalled(QString uid);

    bool appsScanned();
    void scanApps();

    // False (and a log line) if the installer could not be started
    static bool openInstaller(QString path);
private:
    // Get all installed UIDs; false if AppArc could not be read
    bool applications(QStringList &applications) const;
};

#endif // APPMANAGEMENT_H
