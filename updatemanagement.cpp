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

}

int UpdateManagement::updateCount() {
    QSettings settings("WunderWungiel", "Wunderland");
    return settings.value("apps/updateIds").toStringList().count();
}

void UpdateManagement::loadList() {
    if (lastUpdateMoreThanADayAgo() || !widget) {
        QSettings settings("WunderWungiel", "Wunderland");
        QStringList installedIds = settings.value("apps/installedIds").toStringList();

        QueryItems query;
        query << qMakePair(QString("id"), installedIds.join(","))
              << qMakePair(QString("widget"), QString("true"));
        ApiReply *reply = api->get(api->buildUrl("/applist.php", query, 0));
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

        if (checkForUpdate(app.value("uid"), app.value("version"))) {
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

bool UpdateManagement::checkForUpdate(QString uid, QString version) {
    QString installedVersion = appManagement.getVersion(uid);
    return installedVersion.length() > 0 && !version.isEmpty() && appManagement.compareVersions(installedVersion, version);
}
