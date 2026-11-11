#include "appmanagement.h"

AppManagement::AppManagement(QObject *parent) :
    QObject(parent)
{
    // Connected once here; connecting per start made the handler run N times
    connect(&installServerThread, SIGNAL(finished()),
            this, SLOT(receiveThreadSignal()));
}

QString AppManagement::getVersion(QString uid) {
    QString version = "";

    bool ok = false;
    uint uidInt = uid.toUInt(&ok, 16);
    Swi::RSisRegistrySession iSisRegSession;

    if (KErrNone != iSisRegSession.Connect() ) {
        qWarning() << "AppManagement: cannot connect to the SIS registry";
        return version;
    }
    CleanupClosePushL( iSisRegSession );

    //Add here you application UID to access your installed application PKG file.
    const TUid packageId = TUid::Uid((TInt)uidInt);
    Swi::RSisRegistryEntry packageEntry;

    if( KErrNone == packageEntry.Open( iSisRegSession, packageId ) ) {
        CleanupClosePushL( packageEntry );

        const TVersion v = packageEntry.VersionL();
        version = QString::number(v.iMajor) + "." + QString::number(v.iMinor) + "." + QString::number(v.iBuild);

        CleanupStack::PopAndDestroy( &packageEntry );
    }

    CleanupStack::PopAndDestroy( &iSisRegSession );

    return version;
}

bool AppManagement::hasUpdate(QString uid, QString availableVersion) {
    return isUpdate(getVersion(uid), availableVersion);
}

bool AppManagement::isUpdate(QString installedVersion, QString availableVersion) {
    return !installedVersion.isEmpty() && !availableVersion.isEmpty() && compareVersions(installedVersion, availableVersion);
}

bool AppManagement::compareVersions(QString appVersion, QString webVersion) {
    //appVersion -> installed, webVersion -> newest one in the list
    //Returns true if webVersion is newer
    if ((appVersion != webVersion) && (appVersion.length() > 0) && (webVersion.length() > 0)) {
        //Versions differ, check whether it is newer
        int webVersion1, webVersion2, webVersion3;
        webVersion1 = webVersion.mid(0,webVersion.indexOf(".", 0)).toInt();
        webVersion2 = webVersion.mid(webVersion.indexOf(".", 0)+1,webVersion.indexOf(".", webVersion.indexOf(".", 0)+1)-(webVersion.indexOf(".", 0)+1)).toInt();
        webVersion3 = webVersion.mid(webVersion.indexOf(".", webVersion.indexOf(".", 0)+1)+1,webVersion.length()-webVersion.indexOf(".", webVersion.indexOf(".", 0)+1)+1).toInt();

        int appVersion1, appVersion2, appVersion3;
        appVersion1 = appVersion.mid(0,appVersion.indexOf(".", 0)).toInt();
        appVersion2 = appVersion.mid(appVersion.indexOf(".", 0)+1,appVersion.indexOf(".", appVersion.indexOf(".", 0)+1)-(appVersion.indexOf(".", 0)+1)).toInt();
        appVersion3 = appVersion.mid(appVersion.indexOf(".", appVersion.indexOf(".", 0)+1)+1,appVersion.length()-appVersion.indexOf(".", appVersion.indexOf(".", 0)+1)+1).toInt();

        if (appVersion1 < webVersion1) {
            //Major version is newer
            return true;
        } else if (appVersion1 > webVersion1) {
            //Major version is older
            return false;
        } else {
            if (appVersion2 < webVersion2) {
                //Minor version is newer
                return true;
            } else if (appVersion2 > webVersion2) {
                //Minor version is older
                return false;
            } else {
                if (appVersion3 < webVersion3) {
                    //Build number is newer
                    return true;
                } else {
                    //Build number is older or equal
                    return false;
                }
            }
        }
    } else {
        //Versions are identical (or empty), result is false
        return false;
    }
}

bool AppManagement::launchApp(QString uid) {
    bool ok = false;
    uint uidInt = uid.toUInt(&ok, 16);
    const TUid aAppUid = TUid::Uid((TInt)uidInt);

    RApaLsSession apaLsSession;
    User::LeaveIfError(apaLsSession.Connect());
    CleanupClosePushL(apaLsSession);

    TApaAppInfo appInfo;
    TInt retVal = apaLsSession.GetAppInfo(appInfo, aAppUid);

    if(retVal == KErrNone) {
        CApaCommandLine* cmdLine = CApaCommandLine::NewLC();
        cmdLine->SetExecutableNameL(appInfo.iFullName);
        cmdLine->SetCommandL(EApaCommandRun);
        User::LeaveIfError( apaLsSession.StartApp(*cmdLine) );

        CleanupStack::PopAndDestroy(cmdLine);

        ok = true;
    } else {
        // The application not found!
        ok = false;
    }

    CleanupStack::PopAndDestroy(&apaLsSession);

    return ok;
}

bool AppManagement::checkAppInstalled(QString uid) {
    qDebug() << "checkAppInstalled";

    if (uid.length() == 0) return false;

    bool ok = false;
    uint uidInt = uid.toUInt(&ok, 16);
    const TUid aAppUid = TUid::Uid((TInt)uidInt);

    RApaLsSession apaLsSession;
    User::LeaveIfError(apaLsSession.Connect());
    CleanupClosePushL(apaLsSession);

    TApaAppInfo appInfo;
    TInt retVal = apaLsSession.GetAppInfo(appInfo, aAppUid);

    CleanupStack::PopAndDestroy(&apaLsSession);

    if(retVal == KErrNone) {
        return true;
    } else {
        return false;
    }
}


void AppManagement::startInstallServerListening() {
    qDebug() << "listening started";
    installServerThread.start();
}


void AppManagement::openInstaller(QString path) {
    path.replace("/", "\\");
    TPtrC exe(reinterpret_cast<const TText*>(path.data()));

    RApaLsSession apaLsSession;
    CleanupClosePushL(apaLsSession );
    User::LeaveIfError(apaLsSession.Connect());
    TThreadId threadId;
    apaLsSession.StartDocument(exe, threadId);
    CleanupStack::PopAndDestroy(&apaLsSession );
}


// App scanning
bool AppManagement::appsScanned() {
    QSettings settings("WunderWungiel", "Wunderland");
    return settings.value("settings/appsScanned", false).toBool();
}

void AppManagement::scanApps() {
    QStringList apps = applications();

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

QStringList AppManagement::applications() const
{
    RApaLsSession lsSession;
    QStringList applications;

    // Connect to application architecture server
    User::LeaveIfError(lsSession.Connect());
    CleanupClosePushL(lsSession);

    TApaAppInfo appInfo;
    lsSession.GetAllApps();

    while (lsSession.GetNextApp(appInfo) == KErrNone) {
        TApaAppCapabilityBuf capability;
        User::LeaveIfError(lsSession.GetAppCapability(capability,
            appInfo.iUid));
        if (appInfo.iCaption.Length() > 0 && !capability().iAppIsHidden) {
            applications.append("0x"+QString::number((TUint)appInfo.iUid.iUid, 16).toUpper());
        }
    }
    CleanupStack::PopAndDestroy(&lsSession);

    return applications;
}
