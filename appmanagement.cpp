#include "appmanagement.h"
#include "versioncompare.h"
#include "platform/apps.h"

#include <QFile>
#include <QSettings>

AppManagement::AppManagement(QObject *parent) :
    QObject(parent)
{
}

QString AppManagement::getVersion(QString uid) {
    return Platform::installedVersion(uid);
}

bool AppManagement::hasUpdate(QString uid, QString availableVersion) {
    return isNewer(getVersion(uid), availableVersion);
}

bool AppManagement::launchApp(QString uid) {
    return Platform::launchApp(uid);
}

bool AppManagement::checkAppInstalled(QString uid) {
    if (uid.length() == 0) return false;
    return Platform::isAppInstalled(uid);
}

// App scanning
bool AppManagement::appsScanned() {
    QSettings settings;
    return settings.value("settings/appsScanned", false).toBool();
}

void AppManagement::scanApps() {
    QStringList apps;
    // Not marked as scanned, so the next start tries again
    if (!Platform::visibleApps(apps)) return;

    QByteArray liste;
    QString filePath = ":/apps.txt";
    if (QFile::exists(filePath)) {
        QFile file(filePath);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
        liste = file.readAll();
    }

    QStringList installed;
    QSettings settings;
    if (settings.contains("apps/installedIds")) installed = settings.value("apps/installedIds").toStringList();

    for (int i=0; i<apps.count(); i++) {
        int indexOf = liste.indexOf(apps.at(i));
        if (indexOf != -1) {
            indexOf = liste.indexOf("<id>", indexOf)+4;

            QString id = liste.mid(indexOf, (liste.indexOf("</id>", indexOf)-indexOf));
            if (!installed.contains(id)) installed.append(id);
        }
    }

    settings.setValue("apps/installedIds", installed);
    settings.setValue("settings/appsScanned", true);
}

