// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#include "device.h"

#include <QDir>
#include <QStringList>

#ifdef Q_OS_SYMBIAN
#include <QSystemInfo>
#include <QSystemStorageInfo>
#else
#include <QLocale>
#endif

namespace Platform {

#ifdef Q_OS_SYMBIAN

QString osVersion()
{
    QtMobility::QSystemInfo si;
    return si.version(QtMobility::QSystemInfo::Os);
}

QString deviceLanguage()
{
    QtMobility::QSystemInfo si;
    return si.currentLanguage().toLower();
}

QString downloadFolder(qint64 minFree)
{
    // Same folder as the log file (logger.cpp)
    const char *candidates[] = { "F:/download/Wunderland", "E:/download/Wunderland",
                                 "C:/Data/download/Wunderland" };
    QtMobility::QSystemStorageInfo storage;
    const QStringList drives = storage.logicalDrives();
    for (unsigned i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
        const QString path = QString::fromLatin1(candidates[i]);
        const QString drive = path.left(1);
        if (!drives.contains(drive, Qt::CaseInsensitive)) continue;
        if (!QDir(path.left(3)).exists()) continue;
        if (minFree > 0 && storage.availableDiskSpace(drive) <= minFree) continue;
        if (!QDir().mkpath(path)) continue;
        return path;
    }
    return QString();
}

#else // !Q_OS_SYMBIAN

QString osVersion()
{
    return QLatin1String("desktop");
}

QString deviceLanguage()
{
    // "pl_PL" -> "pl"
    return QLocale::system().name().section('_', 0, 0).toLower();
}

QString downloadFolder(qint64)
{
    // Next to the log file (logger.cpp); free space is not checked
    const QString path = QDir::homePath() + QLatin1String("/.wunderland/download");
    if (!QDir().mkpath(path)) return QString();
    return path;
}

#endif // Q_OS_SYMBIAN

} // namespace Platform
