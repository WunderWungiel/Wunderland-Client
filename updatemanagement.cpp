#include "updatemanagement.h"
#include "applistparser.h"
#include <QDebug>

UpdateManagement::UpdateManagement(StoreApi *_api, bool _widget, QObject *parent) :
    QObject(parent)
{
    api = _api;

    widget = _widget;
    if (widget) {
        settingsKey = "widget/lastUpdateTime";
    } else {
        settingsKey = "personal/lastUpdateTime";
    }

    useUnsigned = false;
}

int UpdateManagement::updateCount() {
    QSettings settings("WunderWungiel", "Wunderland");
    return settings.value("apps/updateIds").toStringList().count();
}

void UpdateManagement::loadList() {
    if (lastUpdateMoreThanADayAgo() || !widget) {
        QSettings settings("WunderWungiel", "Wunderland");
        QStringList installedIds = settings.value("apps/installedIds").toStringList();

        useUnsigned = api->useUnsigned();

        QueryItems query;
        query << qMakePair(QString("id"), installedIds.join(","))
              << qMakePair(QString("widget"), QString("true"));
        ApiReply *reply = api->get(api->buildUrl("/applist.php", query, StoreApi::UnsignedParam));
        connect(reply, SIGNAL(finished()),
                this, SLOT(listLoaded()));
    } else {
        if (widget) emit updateWidgetCount(updateCount());
    }
}

void UpdateManagement::listLoaded() {
    ApiReply *reply = qobject_cast<ApiReply *>(sender());
    if (!reply) return; // deletes itself

    // On error keep the last known update list
    if (reply->error() == StoreApi::NoError) parseList(reply->text());
}

void UpdateManagement::parseList(QString html) {
    AppListResult result = AppListParser::parse(html);
    if (!result.ok) {
        // Keep the last known update list
        qWarning() << "UpdateManagement: bad XML:" << result.errorString;
        return;
    }

    QSettings settings("WunderWungiel", "Wunderland");
    int highestSeenId = settings.value("apps/highestSeenId", -1).toInt();
    QStringList updateIds;

    int updateAvailable = 0;
    int highestId = 0;
    int newApps = 0;

    for (int a = 0; a < result.apps.count(); a++) {
        const AppFields &app = result.apps.at(a);

        int id = app.value("id").toInt();
        // ID hasn't been seen
        if (id > highestId) highestId = id;

        QString uid = app.value("uid");
        QString uidstore = app.value("uidstore");
        QString uidunsigned = app.value("uidunsigned");

        QString version = app.value("version");
        QString versionstore = app.value("versionstore");
        QString versionunsigned = app.value("versionunsigned");

        if (checkForUpdate(uid, uidstore, (useUnsigned ? uidunsigned : "update"), version, versionstore, (useUnsigned ? versionunsigned : "update"))) {
            updateAvailable++;
            updateIds.append(QString::number(id));
        }
    }
    qDebug() << "UpdateManagement:" << result.apps.count() << "apps," << updateAvailable << "updates";

    newApps = highestId - highestSeenId;

    settings.setValue(settingsKey, QDateTime::currentDateTime());
    settings.setValue("apps/updateIds", updateIds);
    if (widget) emit loadingFinished(updateAvailable, newApps);
    else emit loadingFinished(updateIds);
}

bool UpdateManagement::lastUpdateMoreThanADayAgo() {
    QSettings settings("WunderWungiel", "Wunderland");
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime lastUpdateTime = settings.value(settingsKey, currentTime).toDateTime();

    if (abs(lastUpdateTime.daysTo(currentTime)) >= 1 || lastUpdateTime == currentTime || !widget) {
        return true;
    } else {
        return false;
    }
}

bool UpdateManagement::checkForUpdate(QString uid, QString uidstore, QString uidunsigned, QString version, QString versionstore, QString versionunsigned) {
    QString installedVersion = "";

    qDebug() << "checkForUpdate: " + uid + " - " + uidunsigned + " - " + versionunsigned;

    if ((installedVersion = appManagement.getVersion(uid)).length() > 0) {
        // Check for selfsigned install if there are updates

        // Check if selfsigned was updated later
        if (!version.isEmpty() && appManagement.compareVersions(installedVersion, version)) return true;
        // Check if unsigned version is newer
        if (!versionunsigned.isEmpty() && appManagement.compareVersions(installedVersion, versionunsigned)) return true;

    }
    if ((installedVersion = appManagement.getVersion(uidunsigned)).length() > 0) {
        // Check for unsigned install if there are updates

        // Check if unsigned version is newer
        if (!versionunsigned.isEmpty() && appManagement.compareVersions(installedVersion, versionunsigned)) return true;
    }
    if ((installedVersion = appManagement.getVersion(uidstore)).length() > 0) {
        // Check for Store version install if there are updates

        // Check if selfsigned was updated later
        if (!version.isEmpty() && appManagement.compareVersions(installedVersion, version)) return true;
        // Check if unsigned version is newer
        if (!versionunsigned.isEmpty() && appManagement.compareVersions(installedVersion, versionunsigned)) return true;
        // Check if Store version is newer
        if (!versionstore.isEmpty() && appManagement.compareVersions(installedVersion, versionstore)) return true;

    }

    return false;
}
