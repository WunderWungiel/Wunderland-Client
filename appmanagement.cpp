#include "appmanagement.h"
#include "versioncompare.h"

// The ...L() helpers may leave. Only the public functions call them, inside
// TRAPD, so a leave turns into a return value and a log line instead of an
// uncaught exception in a Qt slot.
namespace {

TUid uidFromString(const QString &uid)
{
    return TUid::Uid((TInt)uid.toUInt(0, 16));
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

void openInstallerL(const TDesC &file)
{
    RApaLsSession session;
    User::LeaveIfError(session.Connect());
    CleanupClosePushL(session);

    TThreadId threadId;
    User::LeaveIfError(session.StartDocument(file, threadId));

    CleanupStack::PopAndDestroy(&session);
}

// Visible apps (with a caption, not hidden) as "0x<UID>"
void applicationsL(QStringList &applications)
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
            applications.append("0x"+QString::number((TUint)appInfo.iUid.iUid, 16).toUpper());
        }
    }

    CleanupStack::PopAndDestroy(&session);
}

} // namespace

AppManagement::AppManagement(QObject *parent) :
    QObject(parent)
{
}

QString AppManagement::getVersion(QString uid) {
    TVersion v;
    TBool found = EFalse;
    TRAPD(err, found = packageVersionL(uidFromString(uid), v));
    if (err != KErrNone) {
        qWarning() << "AppManagement: cannot read the SIS registry for" << uid << "- error" << err;
        return QString();
    }
    if (!found) return QString();

    return QString::number(v.iMajor) + "." + QString::number(v.iMinor) + "." + QString::number(v.iBuild);
}

bool AppManagement::hasUpdate(QString uid, QString availableVersion) {
    return isNewer(getVersion(uid), availableVersion);
}

bool AppManagement::launchApp(QString uid) {
    TBool found = EFalse;
    TRAPD(err, found = launchAppL(uidFromString(uid)));
    if (err != KErrNone) {
        qWarning() << "AppManagement: cannot launch" << uid << "- error" << err;
        return false;
    }
    if (!found) {
        qWarning() << "AppManagement: cannot launch" << uid << "- not installed";
        return false;
    }
    return true;
}

bool AppManagement::checkAppInstalled(QString uid) {
    if (uid.length() == 0) return false;

    TBool found = EFalse;
    TRAPD(err, found = appInstalledL(uidFromString(uid)));
    if (err != KErrNone) {
        qWarning() << "AppManagement: cannot check if" << uid << "is installed - error" << err;
        return false;
    }
    return found;
}


bool AppManagement::openInstaller(QString path) {
    path.replace("/", "\\");
    TPtrC file(reinterpret_cast<const TText*>(path.constData()), path.length());

    TRAPD(err, openInstallerL(file));
    if (err != KErrNone) {
        qWarning() << "AppManagement: cannot open the installer for" << path << "- error" << err;
        return false;
    }
    return true;
}


// App scanning
bool AppManagement::appsScanned() {
    QSettings settings("WunderWungiel", "Wunderland");
    return settings.value("settings/appsScanned", false).toBool();
}

void AppManagement::scanApps() {
    QStringList apps;
    // Not marked as scanned, so the next start tries again
    if (!applications(apps)) return;

    QByteArray liste;
    QString filePath = ":/apps.txt";
    if (QFile::exists(filePath)) {
        QFile file(filePath);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
        liste = file.readAll();
    }

    QStringList installed;
    QSettings settings("WunderWungiel", "Wunderland");
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

bool AppManagement::applications(QStringList &applications) const
{
    TRAPD(err, applicationsL(applications));
    if (err != KErrNone) {
        qWarning() << "AppManagement: cannot list the installed apps - error" << err;
        applications.clear();
        return false;
    }
    return true;
}
