// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appmanagement.h"
#include "versioncompare.h"
#include "platform/apps.h"

#include <QFile>

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

bool AppManagement::installedStoreIds(QStringList &ids) {
    QStringList apps;
    if (!Platform::visibleApps(apps)) return false;

    QByteArray liste;
    QString filePath = ":/apps.txt";
    if (QFile::exists(filePath)) {
        QFile file(filePath);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
        liste = file.readAll();
    }

    for (int i=0; i<apps.count(); i++) {
        int indexOf = liste.indexOf(apps.at(i));
        if (indexOf != -1) {
            indexOf = liste.indexOf("<id>", indexOf)+4;

            QString id = liste.mid(indexOf, (liste.indexOf("</id>", indexOf)-indexOf));
            if (!ids.contains(id)) ids.append(id);
        }
    }
    return true;
}

