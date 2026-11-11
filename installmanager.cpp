#include "installmanager.h"
#include "appmanagement.h"
#include "updatetracker.h"
#include "language.h"
#include "downloader.h"
#include "versioncompare.h"
#include <QDebug>

InstallManager::InstallManager(StoreApi *api, AppManagement *appManagement, UpdateTracker *updateTracker,
                               Language *language, QObject *parent) :
    QObject(parent),
    m_appManagement(appManagement),
    m_updateTracker(updateTracker),
    m_language(language),
    m_state(Idle),
    m_progress(0)
{
    m_downloader = new Downloader(api, this);
    connect(m_downloader, SIGNAL(progressChanged(int)),
            this, SLOT(onProgress(int)));
    connect(m_downloader, SIGNAL(downloadDone()),
            this, SLOT(onDownloadDone()));
    connect(m_downloader, SIGNAL(downloadError(int)),
            this, SLOT(onDownloadError(int)));
    // Connected once here; connecting per download made the handler run N times
    connect(&m_installServerThread, SIGNAL(finished()),
            this, SLOT(onInstallerClosed()));
}

int InstallManager::buttonAction(const QVariant &app) {
    QVariantMap item = app.toMap();

    if (m_state != Idle && m_app.value("id").toInt() == item.value("id").toInt())
        return Downloading;

    // Read the registry, not the installed role: item is the snapshot the page was opened
    // with, and the app may have been installed or removed since
    QString installedVersion = m_appManagement->getVersion(item.value("uid").toString());
    if (installedVersion.isEmpty()) return Download;
    if (isNewer(installedVersion, item.value("version").toString())) return Update;
    return Launch;
}

bool InstallManager::buttonPressed(const QVariant &app, int action) {
    QVariantMap item = app.toMap();

    setProgress(0);

    if (action == Download || action == Update) {
        // Set first: a failed start emits downloadError right away, which resets it
        m_app = item;
        setState(Transferring);
        m_downloader->startDownload(item.value("download").toString(), item.value("downloadsize").toLongLong(),
                                    QString::number(item.value("id").toInt()), m_language->currentLanguage());
        return m_state != Idle;
    } else if (action == Launch) {
        m_appManagement->launchApp(item.value("uid").toString());
    }

    // return if download progress shall be displayed
    return true;
}

bool InstallManager::cancelDownload() {
    if (!m_downloader->cancelDownload()) return false;
    m_app.clear();
    setState(Idle);
    return true;
}

void InstallManager::onProgress(int percent) {
    setProgress(percent);
}

void InstallManager::onDownloadDone() {
    qDebug() << "InstallManager: download done, waiting for the installer";
    setState(Installing);
    m_installServerThread.start();
}

void InstallManager::onDownloadError(int code) {
    m_app.clear();
    setState(Idle);
    emit downloadError(code);
}

void InstallManager::onInstallerClosed() {
    m_downloader->deleteFile();

    // Installation was successful if the downloaded version is now installed
    int id = m_app.value("id").toInt();
    QString installedVersion = m_appManagement->getVersion(m_app.value("uid").toString());
    bool success = !installedVersion.isEmpty()
            && installedVersion == m_app.value("version").toString();
    qDebug() << "InstallManager: installer closed, app" << id << (success ? "installed" : "not installed");

    m_updateTracker->installFinished(id, success);
    if (success) emit appInstalled(id);

    m_app.clear();
    setState(Idle);

    emit installServerFinished(success ? 0 : 1);
}

void InstallManager::setState(State state) {
    if (state == m_state) return;
    m_state = state;
    emit stateChanged();
}

void InstallManager::setProgress(int percent) {
    m_progress = percent;
    emit progressChanged();
}
