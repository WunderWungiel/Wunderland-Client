#include "updatetracker.h"
#include "appmanagement.h"
#include "applistparser.h"
#include "versioncompare.h"
#include <QSettings>
#include <QDateTime>
#include <QDebug>

static const char *LastCheckKey = "widget/lastUpdateTime";

UpdateTracker::UpdateTracker(StoreApi *api, AppManagement *appManagement, QObject *parent) :
    QObject(parent), m_api(api), m_appManagement(appManagement)
{
    QSettings settings("WunderWungiel", "Wunderland");
    m_updateIds = settings.value("apps/updateIds").toStringList();
    m_installedIds = settings.value("apps/installedIds").toStringList();
    m_highestSeenId = settings.value("apps/highestSeenId", 0).toInt();
}

void UpdateTracker::check() {
    QueryItems query;
    query << qMakePair(QString("id"), m_installedIds.join(","))
          << qMakePair(QString("widget"), QString("true"));
    ApiReply *reply = m_api->get(m_api->buildUrl("/applist.php", query, 0));
    connect(reply, SIGNAL(finished()),
            this, SLOT(checkLoaded()));
}

void UpdateTracker::checkLoaded() {
    ApiReply *reply = qobject_cast<ApiReply *>(sender());
    if (!reply) return; // deletes itself

    // On error keep the last known update list
    if (reply->error() != StoreApi::NoError) return;

    AppListResult result = AppListParser::parse(reply->text());
    if (!result.ok) {
        qWarning() << "UpdateTracker: bad XML:" << result.errorString;
        return;
    }

    QStringList updateIds;
    int highestId = 0;
    for (int a = 0; a < result.apps.count(); a++) {
        const AppEntry &app = result.apps.at(a);
        if (app.id > highestId) highestId = app.id;
        if (m_appManagement->hasUpdate(app.uid, app.version)) updateIds.append(QString::number(app.id));
    }
    qDebug() << "UpdateTracker:" << result.apps.count() << "apps," << updateIds.count() << "updates";

    QSettings settings("WunderWungiel", "Wunderland");
    settings.setValue(LastCheckKey, QDateTime::currentDateTime());
    setUpdateIds(updateIds);

    emit checkFinished(updateIds.count(), highestId - m_highestSeenId);
}

bool UpdateTracker::lastCheckOlderThanADay() const {
    QSettings settings("WunderWungiel", "Wunderland");
    if (!settings.contains(LastCheckKey)) return true;
    QDateTime lastCheck = settings.value(LastCheckKey).toDateTime();
    return qAbs(lastCheck.daysTo(QDateTime::currentDateTime())) >= 1;
}

bool UpdateTracker::track(AppEntry &app) {
    QString id = QString::number(app.id);

    if (app.id > m_highestSeenId) {
        m_highestSeenId = app.id;
        QSettings settings("WunderWungiel", "Wunderland");
        settings.setValue("apps/highestSeenId", m_highestSeenId);
    }

    QString installedVersion = m_appManagement->getVersion(app.uid);
    app.installed = !installedVersion.isEmpty();

    if (app.installed && !m_installedIds.contains(id)) {
        // Not tracked yet, so check() has not looked at it
        if (isNewer(installedVersion, app.version)) {
            app.updateAvailable = true;
            setUpdateIds(QStringList(m_updateIds) << id);
        }
        m_installedIds.append(id);
        saveInstalledIds();
    } else if (m_updateIds.contains(id)) {
        app.updateAvailable = true;
    }

    if (!app.installed && m_installedIds.contains(id)) {
        // Removed since
        m_installedIds.removeAll(id);
        saveInstalledIds();
        return false;
    }
    return true;
}

void UpdateTracker::installFinished(int id, bool success) {
    QString idString = QString::number(id);

    m_installedIds.removeAll(idString);
    m_installedIds.append(idString);
    saveInstalledIds();

    if (success && m_updateIds.contains(idString)) {
        QStringList updateIds = m_updateIds;
        updateIds.removeAll(idString);
        setUpdateIds(updateIds);
    }
}

void UpdateTracker::setUpdateIds(const QStringList &ids) {
    QSettings settings("WunderWungiel", "Wunderland");
    settings.setValue("apps/updateIds", ids);
    if (ids == m_updateIds) return;
    m_updateIds = ids;
    emit countChanged();
}

void UpdateTracker::saveInstalledIds() {
    QSettings settings("WunderWungiel", "Wunderland");
    settings.setValue("apps/installedIds", m_installedIds);
}
