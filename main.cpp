#include <QtGui/QApplication>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include "qmlapplicationviewer.h"

#include "getlist.h"
#include "roleitemmodel.h"
#include "hswidget.h"
#include "news.h"
#include "update.h"
#include "language.h"
#include "subscriber.h"
#include "settings.h"
#include "symbian.h"
#include "logger.h"

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

    qDebug() << "registerWidgetBefore";
    HSWidget widget;
    widget.registerWidget();
    qDebug() << "registerWidgetAfter";

    AppManagement appManagement;
    if (!appManagement.appsScanned()) appManagement.scanApps();

    News news;
    Update update;
    Language *language = new Language();
    Subscriber subscriber;
    Settings settings;
    GetList getList(language);
    Symbian symbianFunctions;

    app->installEventFilter(&getList);

    QmlApplicationViewer viewer;

    qmlRegisterType<RoleItemModel>("com.applist.RoleItemModel", 1, 0, "RoleItemModel");

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
