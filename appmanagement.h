#ifndef APPMANAGEMENT_H
#define APPMANAGEMENT_H

#include <QObject>
#include <installserverthread.h>
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
    static bool compareVersions(QString appVersion, QString webVersion);

    // True if the app with this UID is installed in a version older than availableVersion
    bool hasUpdate(QString uid, QString availableVersion);
    // The same check for an installed version already read with getVersion()
    // (empty = not installed), so callers that need it too don't read the registry twice
    static bool isUpdate(QString installedVersion, QString availableVersion);

    bool launchApp(QString uid);
    Q_INVOKABLE bool checkAppInstalled(QString uid);
    Q_INVOKABLE void startInstallServerListening();

    bool appsScanned();
    void scanApps();

    static void openInstaller(QString path);
signals:
    void installServerFinished();
public slots:
    void receiveThreadSignal() {
        qDebug() << "receiveThreadSignal";
        emit installServerFinished();
    }

private:
    InstallServerThread installServerThread;

    // Get all installed UIDs
    QStringList applications() const;
};

#endif // APPMANAGEMENT_H
