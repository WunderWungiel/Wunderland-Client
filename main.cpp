#include <QtGui/QApplication>
#include <QTimer>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QNetworkAccessManager>
#include "qmlapplicationviewer.h"

#include "appcatalog.h"
#include "installmanager.h"
#include "updatetracker.h"
#include "appmanagement.h"
#include "newsmodel.h"
#include "applistmodel.h"
#ifdef Q_OS_SYMBIAN
#include "platform/hswidget.h"
#endif
#include "news.h"
#include "update.h"
#include "language.h"
#include "platform/subscriber.h"
#include "settings.h"
#include "symbian.h"
#include "logger.h"
#include "storeapi.h"

#include <QDebug>

// APP_Version is defined by Wunderland.pro as a bare token (2.0.5)
#define WL_STRINGIFY2(x) #x
#define WL_STRINGIFY(x) WL_STRINGIFY2(x)

// The version as the UI shows it, Symbian style: "2.0.5" -> "2.00(5)"
static QString displayVersion(const QString &version)
{
    const QStringList parts = version.split(QLatin1Char('.'));
    if (parts.size() != 3)
        return version;
    return QString::fromLatin1("%1.%2(%3)")
            .arg(parts.at(0))
            .arg(parts.at(1), 2, QLatin1Char('0'))
            .arg(parts.at(2));
}

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    // Before anything uses QSettings, so QSettings() needs no arguments
    app->setOrganizationName(QLatin1String("WunderWungiel"));
    app->setApplicationName(QLatin1String("Wunderland"));
    app->setApplicationVersion(QLatin1String(WL_STRINGIFY(APP_Version)));

    Logger::install();
    qDebug() << "Wunderland" << app->applicationVersion() << "starting";
    qDebug() << "Log file:" << Logger::instance()->logFilePath();

    // Manage, if an app has started AppList
    int launchAppId = -1;
    const QStringList list = app->arguments();
    for (int i=0; i<list.length(); i++) {
        if (list.at(i).startsWith("appListId:")) {
            launchAppId = list.at(i).mid(10).toInt();
            break;
        }
    }

    // Before the widget, so its texts are translated; declared before its users so it outlives them
    Language language;

    // The one network manager; declared before its users so it outlives them
    QNetworkAccessManager networkManager;
    StoreApi storeApi(&networkManager, &language);

    AppManagement appManagement;
    // On the first run it also maps the installed apps to store IDs
    UpdateTracker updateTracker(&storeApi, &appManagement);

#ifdef Q_OS_SYMBIAN
    // The homescreen widget only exists on Symbian; registered once the UI is shown (below)
    HSWidget widget(&updateTracker);
#endif

    News news(&storeApi);
    Update update(&storeApi);
    Subscriber subscriber;
    Settings settings;
    AppCatalog appCatalog(&storeApi, &updateTracker);
    InstallManager installManager(&storeApi, &appManagement, &updateTracker, &language);
    QObject::connect(&installManager, SIGNAL(appInstalled(int)),
                     &appCatalog, SLOT(markInstalled(int)));
    updateTracker.check();
    Symbian symbianFunctions;

    QmlApplicationViewer viewer;

    // Not creatable from QML; registered so the model properties can be read
    qmlRegisterType<AppListModel>();
    qmlRegisterType<NewsModel>();
    // Only for the enums (InstallManager.Download, ...); the instance is the installManager property
    qmlRegisterUncreatableType<InstallManager>("Wunderland", 1, 0, "InstallManager", "Use the installManager context property");
    // Only for the enums: AppCatalog.SearchList, StoreApi.NetworkError, ...
    qmlRegisterUncreatableType<AppCatalog>("Wunderland", 1, 0, "AppCatalog", "Use the appCatalog context property");
    qmlRegisterUncreatableType<StoreApi>("Wunderland", 1, 0, "StoreApi", "Not used from QML directly");

    viewer.rootContext()->setContextProperty("appCatalog", &appCatalog);
    viewer.rootContext()->setContextProperty("installManager", &installManager);
    viewer.rootContext()->setContextProperty("updateTracker", &updateTracker);
    viewer.rootContext()->setContextProperty("appManagement", &appManagement);
    viewer.rootContext()->setContextProperty("news", &news);
    viewer.rootContext()->setContextProperty("update", &update);
    viewer.rootContext()->setContextProperty("l", &language);
    viewer.rootContext()->setContextProperty("subscriber", &subscriber);
    viewer.rootContext()->setContextProperty("settings", &settings);
    viewer.rootContext()->setContextProperty("symbianFunctions", &symbianFunctions);
    viewer.rootContext()->setContextProperty("logger", Logger::instance());

    viewer.rootContext()->setContextProperty("launchAppId", launchAppId);
    viewer.rootContext()->setContextProperty("appVersion", displayVersion(app->applicationVersion()));

    viewer.setSource(QUrl("qrc:/qml/Wunderland/MainAppLoader.qml"));
    viewer.showExpanded();

    viewer.raise();

#ifdef Q_OS_SYMBIAN
    // Registering is slow; after a delay it no longer holds up the splash screen
    QTimer::singleShot(1000, &widget, SLOT(registerWidget()));
#endif

    return app->exec();
}
