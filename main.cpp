#include <QtGui/QApplication>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QNetworkAccessManager>
#include "qmlapplicationviewer.h"

#include "appcatalog.h"
#include "installmanager.h"
#include "updatetracker.h"
#include "appmanagement.h"
#include "roleitemmodel.h"
#include "applistmodel.h"
#include "hswidget.h"
#include "news.h"
#include "update.h"
#include "language.h"
#include "subscriber.h"
#include "settings.h"
#include "symbian.h"
#include "logger.h"
#include "storeapi.h"

#include <QDebug>

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    app->setApplicationVersion(APP_Version);

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

    // Before the widget, so its texts are translated. Never deleted.
    Language *language = new Language();

    // The one network manager; declared before its users so it outlives them
    QNetworkAccessManager networkManager;
    StoreApi storeApi(&networkManager, language);

    AppManagement appManagement;
    if (!appManagement.appsScanned()) appManagement.scanApps();

    // After scanApps(), which fills apps/installedIds on the first run
    UpdateTracker updateTracker(&storeApi, &appManagement);

    qDebug() << "registerWidgetBefore";
    HSWidget widget(&updateTracker);
    widget.registerWidget();
    qDebug() << "registerWidgetAfter";

    News news(&storeApi);
    Update update(&storeApi);
    Subscriber subscriber;
    Settings settings;
    AppCatalog appCatalog(&storeApi, &updateTracker);
    InstallManager installManager(&storeApi, &appManagement, &updateTracker, language);
    QObject::connect(&installManager, SIGNAL(appInstalled(int)),
                     &appCatalog, SLOT(markInstalled(int)));
    updateTracker.check();
    Symbian symbianFunctions;

    QmlApplicationViewer viewer;

    // Not creatable from QML; registered so the model properties can be read
    qmlRegisterType<AppListModel>();
    qmlRegisterType<RoleItemModel>();
    // Only for the enums (InstallManager.Download, ...); the instance is the installManager property
    qmlRegisterUncreatableType<InstallManager>("Wunderland", 1, 0, "InstallManager", "Use the installManager context property");

    viewer.rootContext()->setContextProperty("appCatalog", &appCatalog);
    viewer.rootContext()->setContextProperty("installManager", &installManager);
    viewer.rootContext()->setContextProperty("updateTracker", &updateTracker);
    viewer.rootContext()->setContextProperty("appManagement", &appManagement);
    viewer.rootContext()->setContextProperty("news", &news);
    viewer.rootContext()->setContextProperty("update", &update);
    viewer.rootContext()->setContextProperty("l", language);
    viewer.rootContext()->setContextProperty("subscriber", &subscriber);
    viewer.rootContext()->setContextProperty("settings", &settings);
    viewer.rootContext()->setContextProperty("symbianFunctions", &symbianFunctions);
    viewer.rootContext()->setContextProperty("logger", Logger::instance());

    viewer.rootContext()->setContextProperty("launchAppId", launchAppId);

    viewer.setSource(QUrl("qrc:/qml/Wunderland/MainAppLoader.qml"));
    //viewer.setMainQmlFile(QLatin1String("qml/Wunderland/MainAppLoader.qml"));
    viewer.showExpanded();

    viewer.raise();

    return app->exec();
}
