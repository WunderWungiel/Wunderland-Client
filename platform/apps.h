#ifndef PLATFORM_APPS_H
#define PLATFORM_APPS_H

#include <QString>
#include <QStringList>

// Installed apps and launching things: the SIS registry and AppArc on Symbian.
// UIDs are hex strings ("0xE9320736"). Errors are logged and turned into the
// "not found"/false result; nothing here throws or leaves.
// On other platforms nothing is installed and nothing can be launched, except
// URLs (QDesktopServices).
namespace Platform {

// Version of the installed package with this UID as "major.minor.build",
// empty if it is not installed
QString installedVersion(const QString &uid);

// True if AppArc knows an app with this UID
bool isAppInstalled(const QString &uid);

// False if the app is not installed or could not be started
bool launchApp(const QString &uid);

// Visible apps (with a caption, not hidden) as "0x<UID>" in upper case.
// False if the list could not be read (uids is then empty).
bool visibleApps(QStringList &uids);

// Opens the .sis at path in the system installer. False if it could not be
// started. installerThreadId (if given) gets the installer's main thread id,
// for InstallerWatcher, or 0.
bool openInstaller(const QString &path, quint64 *installerThreadId = 0);

// Opens url in the browser selected in the phone settings (Qt's
// openUrlExternally always uses the built-in one). False if it could not be started.
bool openUrlInBrowser(const QString &url);

} // namespace Platform

#endif // PLATFORM_APPS_H
