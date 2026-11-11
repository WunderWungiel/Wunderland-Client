// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INSTALLMANAGER_H
#define INSTALLMANAGER_H

#include <QObject>
#include <QVariant>
#include "platform/installerwatcher.h"

class StoreApi;
class AppManagement;
class UpdateTracker;
class Language;
class Downloader;

// Download -> installer -> wait for the installer to close -> check the result,
// for one app at a time.
class InstallManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int state READ state NOTIFY stateChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_ENUMS(Action State)

public:
    // What the detail page button does for an app (QML: InstallManager.Download, ...)
    enum Action {
        Download,     // not installed
        Update,       // installed, the store has a newer version
        Launch,       // installed and up to date
        Downloading   // this app is being downloaded or installed now
    };

    enum State {
        Idle,
        Transferring, // the SIS file is being downloaded
        Installing    // the installer is open
    };

    InstallManager(StoreApi *api, AppManagement *appManagement, UpdateTracker *updateTracker,
                   Language *language, QObject *parent = 0);

    int state() const { return m_state; }
    // Download progress in percent
    int progress() const { return m_progress; }

    // app: a row as AppListModel::get() returns it (DetailPage's detailItem).
    // Returns an Action (int, so QtQuick 1.1 reads it)
    Q_INVOKABLE int buttonAction(const QVariant &app);
    // Does the Action for app; returns whether the download progress is shown
    Q_INVOKABLE bool buttonPressed(const QVariant &app, int action);
    Q_INVOKABLE bool cancelDownload();

signals:
    void stateChanged();
    void progressChanged();
    void downloadError(int code); // a Downloader::ErrorCode
    // statusCode: 0 the app is installed in the downloaded version, 1 failed or aborted
    void installServerFinished(int statusCode);
    // The app with this store id was installed (the lists update their rows)
    void appInstalled(int id);

private slots:
    void onProgress(int percent);
    void onDownloadDone();
    void onDownloadError(int code);
    void onInstallerClosed();

private:
    void setState(State state);
    void setProgress(int percent);

    AppManagement *m_appManagement;
    UpdateTracker *m_updateTracker;
    Language *m_language;
    Downloader *m_downloader;
    InstallerWatcher m_installerWatcher;

    State m_state;
    int m_progress;
    QVariantMap m_app; // the app being downloaded or installed
};

#endif // INSTALLMANAGER_H
