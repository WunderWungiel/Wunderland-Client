// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#include "apps.h"

#include <QDebug>

#ifdef Q_OS_SYMBIAN
#include "sisregistrysession.h"
#include "sisregistryentry.h"
#include <apgcli.h>   // RApaLsSession
#include <apacmdln.h> // CApaCommandLine
#include <apgtask.h>  // TApaTaskList
#include <eikenv.h>   // CEikonEnv
#else
#include <QDesktopServices>
#include <QUrl>
#endif

#ifdef Q_OS_SYMBIAN

// The ...L() helpers may leave. Only the functions of the Platform namespace
// call them, inside TRAPD, so a leave turns into a return value and a log line
// instead of an uncaught exception in a Qt slot.
namespace {

TUid uidFromString(const QString &uid)
{
    return TUid::Uid((TInt)uid.toUInt(0, 16));
}

TPtrC toPtr(const QString &s)
{
    return TPtrC(reinterpret_cast<const TText *>(s.constData()), s.length());
}

// False if no package with this UID is in the SIS registry
TBool packageVersionL(TUid packageId, TVersion &version)
{
    Swi::RSisRegistrySession session;
    User::LeaveIfError(session.Connect());
    CleanupClosePushL(session);

    Swi::RSisRegistryEntry entry;
    TBool found = entry.Open(session, packageId) == KErrNone;
    if (found) {
        CleanupClosePushL(entry);
        version = entry.VersionL();
        CleanupStack::PopAndDestroy(&entry);
    }

    CleanupStack::PopAndDestroy(&session);
    return found;
}

// False if AppArc doesn't know an app with this UID
TBool appInstalledL(TUid appUid)
{
    RApaLsSession session;
    User::LeaveIfError(session.Connect());
    CleanupClosePushL(session);

    TApaAppInfo appInfo;
    TBool found = session.GetAppInfo(appInfo, appUid) == KErrNone;

    CleanupStack::PopAndDestroy(&session);
    return found;
}

// False if AppArc doesn't know an app with this UID
TBool launchAppL(TUid appUid)
{
    RApaLsSession session;
    User::LeaveIfError(session.Connect());
    CleanupClosePushL(session);

    TApaAppInfo appInfo;
    TBool found = session.GetAppInfo(appInfo, appUid) == KErrNone;
    if (found) {
        CApaCommandLine *cmdLine = CApaCommandLine::NewLC();
        cmdLine->SetExecutableNameL(appInfo.iFullName);
        cmdLine->SetCommandL(EApaCommandRun);
        User::LeaveIfError(session.StartApp(*cmdLine));
        CleanupStack::PopAndDestroy(cmdLine);
    }

    CleanupStack::PopAndDestroy(&session);
    return found;
}

void openInstallerL(const TDesC &file, TThreadId &threadId)
{
    RApaLsSession session;
    User::LeaveIfError(session.Connect());
    CleanupClosePushL(session);

    User::LeaveIfError(session.StartDocument(file, threadId));

    CleanupStack::PopAndDestroy(&session);
}

void visibleAppsL(QStringList &uids)
{
    RApaLsSession session;
    User::LeaveIfError(session.Connect());
    CleanupClosePushL(session);

    User::LeaveIfError(session.GetAllApps());

    TApaAppInfo appInfo;
    while (session.GetNextApp(appInfo) == KErrNone) {
        TApaAppCapabilityBuf capability;
        // Skip an app whose capabilities can't be read instead of giving up on all
        if (session.GetAppCapability(capability, appInfo.iUid) != KErrNone) continue;
        if (appInfo.iCaption.Length() > 0 && !capability().iAppIsHidden) {
            uids.append("0x" + QString::number((TUint)appInfo.iUid.iUid, 16).toUpper());
        }
    }

    CleanupStack::PopAndDestroy(&session);
}

// Starts the browser with the URL, or hands the URL to it if it is running.
// Based on CBrowserAppUi::LaunchBrowserL:
// http://www.developer.nokia.com/Community/Wiki/Launch_default_web_browser_using_Symbian_C%2B%2B
// http://www.developer.nokia.com/Community/Wiki/How_to_Launch_Browser_in_3rd_edition
// If the Symbian Web Browser is already open, this needs the SwEvent capability:
// https://bugreports.qt-project.org/browse/QTBUG-15725
void launchBrowserL(const TDesC &url, TUid browserUid)
{
    TApaTaskList taskList(CEikonEnv::Static()->WsSession());
    TApaTask task = taskList.FindApp(browserUid);
    if (task.Exists()) {
        HBufC8 *param = HBufC8::NewLC(url.Length() + 2);
        // "4 " starts/continues the browser with a URL
        param->Des().Append(_L("4 "));
        param->Des().Append(url);
        task.SendMessage(TUid::Uid(0), *param); // the UID is not used
        CleanupStack::PopAndDestroy(param);
    } else {
        HBufC16 *param = HBufC16::NewLC(url.Length() + 2);
        param->Des().Append(_L("4 "));
        param->Des().Append(url);
        RApaLsSession session;
        User::LeaveIfError(session.Connect());
        CleanupClosePushL(session);
        TThreadId id;
        User::LeaveIfError(session.StartDocument(*param, browserUid, id));
        CleanupStack::PopAndDestroy(2, param); // session, param
    }
}

void openUrlL(const TDesC &url)
{
    RApaLsSession session;
    User::LeaveIfError(session.Connect());
    CleanupClosePushL(session);
    // The default app for application/x-web-browse (Settings > Program
    // settings > Default programs > Internet)
    TDataType mimeType(_L8("application/x-web-browse"));
    TUid handlerUid = KNullUid;
    TInt err = session.AppForDataType(mimeType, handlerUid);
    // There may be no handler (S60 3.x): use the built-in browser
    if (err != KErrNone || handlerUid.iUid == 0) {
        const TInt KWmlBrowserUid = 0x10008D39;
        handlerUid = TUid::Uid(KWmlBrowserUid);
    }
    launchBrowserL(url, handlerUid);
    CleanupStack::PopAndDestroy(&session);
}

} // namespace

namespace Platform {

QString installedVersion(const QString &uid)
{
    TVersion v;
    TBool found = EFalse;
    TRAPD(err, found = packageVersionL(uidFromString(uid), v));
    if (err != KErrNone) {
        qWarning() << "Platform: cannot read the SIS registry for" << uid << "- error" << err;
        return QString();
    }
    if (!found) return QString();

    return QString::number(v.iMajor) + "." + QString::number(v.iMinor) + "." + QString::number(v.iBuild);
}

bool isAppInstalled(const QString &uid)
{
    TBool found = EFalse;
    TRAPD(err, found = appInstalledL(uidFromString(uid)));
    if (err != KErrNone) {
        qWarning() << "Platform: cannot check if" << uid << "is installed - error" << err;
        return false;
    }
    return found;
}

bool launchApp(const QString &uid)
{
    TBool found = EFalse;
    TRAPD(err, found = launchAppL(uidFromString(uid)));
    if (err != KErrNone) {
        qWarning() << "Platform: cannot launch" << uid << "- error" << err;
        return false;
    }
    if (!found) {
        qWarning() << "Platform: cannot launch" << uid << "- not installed";
        return false;
    }
    return true;
}

bool visibleApps(QStringList &uids)
{
    uids.clear();
    TRAPD(err, visibleAppsL(uids));
    if (err != KErrNone) {
        qWarning() << "Platform: cannot list the installed apps - error" << err;
        uids.clear();
        return false;
    }
    return true;
}

bool openInstaller(const QString &path, quint64 *installerThreadId)
{
    if (installerThreadId) *installerThreadId = 0;
    QString nativePath = path;
    nativePath.replace("/", "\\");

    TThreadId threadId;
    TRAPD(err, openInstallerL(toPtr(nativePath), threadId));
    if (err != KErrNone) {
        qWarning() << "Platform: cannot open the installer for" << nativePath << "- error" << err;
        return false;
    }
    if (installerThreadId) *installerThreadId = threadId.Id();
    return true;
}

bool openUrlInBrowser(const QString &url)
{
    TRAPD(err, openUrlL(toPtr(url)));
    if (err != KErrNone) {
        qWarning() << "Platform: cannot open" << url << "in the browser - error" << err;
        return false;
    }
    return true;
}

} // namespace Platform

#else // !Q_OS_SYMBIAN

namespace Platform {

QString installedVersion(const QString &)
{
    return QString();
}

bool isAppInstalled(const QString &)
{
    return false;
}

bool launchApp(const QString &uid)
{
    qWarning() << "Platform: cannot launch" << uid << "- not supported on this platform";
    return false;
}

bool visibleApps(QStringList &uids)
{
    uids.clear();
    return true;
}

bool openInstaller(const QString &path, quint64 *installerThreadId)
{
    if (installerThreadId) *installerThreadId = 0;
    qWarning() << "Platform: cannot install" << path << "- not supported on this platform";
    return false;
}

bool openUrlInBrowser(const QString &url)
{
    if (!QDesktopServices::openUrl(QUrl(url))) {
        qWarning() << "Platform: cannot open" << url << "in the browser";
        return false;
    }
    return true;
}

} // namespace Platform

#endif // Q_OS_SYMBIAN
