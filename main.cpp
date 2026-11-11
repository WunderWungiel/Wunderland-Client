#include <QtGui/QApplication>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QNetworkAccessManager>
#include "qmlapplicationviewer.h"

#include "getlist.h"
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

    qDebug() << "registerWidgetBefore";
    HSWidget widget(&storeApi);
    widget.registerWidget();
    qDebug() << "registerWidgetAfter";

    AppManagement appManagement;
    if (!appManagement.appsScanned()) appManagement.scanApps();

    News news(&storeApi);
    Update update(&storeApi);
    Subscriber subscriber;
    Settings settings;
    GetList getList(&storeApi, language);
    Symbian symbianFunctions;

    QmlApplicationViewer viewer;

    // Not creatable from QML; registered so the model properties can be read
    qmlRegisterType<AppListModel>();
    qmlRegisterType<RoleItemModel>();
    // Only for the Action enum (GetList.Download, ...); the instance is the getList property
    qmlRegisterUncreatableType<GetList>("Wunderland", 1, 0, "GetList", "Use the getList context property");

    viewer.rootContext()->setContextProperty("getList", &getList);
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
