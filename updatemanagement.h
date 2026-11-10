#ifndef UPDATEMANAGEMENT_H
#define UPDATEMANAGEMENT_H

#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QDateTime>
#include <QDebug>
#include "appmanagement.h"
#include "storeapi.h"

class UpdateManagement : public QObject
{
    Q_OBJECT
public:
    UpdateManagement(StoreApi *api, bool _widget, QObject *parent = 0);

    void loadList();

    // Update count sync for widget
    int updateCount();

    bool checkForUpdate(QString uid, QString version);
    
signals:
    void loadingFinished(int updates, int newApps);
    void loadingFinished(QStringList updateIds);
    void updateWidgetCount(int updates);
    
public slots:
    void listLoaded();

private:
    AppManagement appManagement;
    StoreApi *api;
    void parseList(QString html);

    bool widget;

    QString settingsKey;
    bool lastUpdateMoreThanADayAgo();
    
};

#endif // UPDATEMANAGEMENT_H
