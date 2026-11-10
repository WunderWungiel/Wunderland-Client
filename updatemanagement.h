#ifndef UPDATEMANAGEMENT_H
#define UPDATEMANAGEMENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <QStringList>
#include <QDateTime>
#include <QDebug>
#include "appmanagement.h"

class UpdateManagement : public QObject
{
    Q_OBJECT
public:
    explicit UpdateManagement(bool _widget, QObject *parent = 0);

    void loadList();

    // Update count sync for widget
    int updateCount();

    bool checkForUpdate(QString uid, QString uidstore, QString uidunsigned, QString version, QString versionstore, QString versionunsigned);
    
signals:
    void loadingFinished(int updates, int newApps);
    void loadingFinished(QStringList updateIds);
    void updateWidgetCount(int updates);
    
public slots:
    void listLoaded();

private:
    AppManagement appManagement;
    QNetworkAccessManager* n_manager;
    void parseList(QString html);
    QString parseItem(QString html, int& lastPos, QString itemName);

    bool widget, useUnsigned;

    QString settingsKey;
    bool lastUpdateMoreThanADayAgo();

    // Returns the redirect target of a finished reply, resolved against the request URL.
    // Returns an empty URL if there is no redirect, if it points back to the same URL, or
    // if the hop limit is reached (the hop count is kept in the reply's "redirects" property).
    QUrl redirectUrl(const QNetworkReply* reply) const {
        QUrl target = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (target.isEmpty()) return QUrl();
        target = reply->url().resolved(target);
        if (target == reply->url()) return QUrl();
        if (reply->property("redirects").toInt() >= 5) {
            qWarning() << "Network: too many redirects, giving up at" << reply->url().toString();
            return QUrl();
        }
        return target;
    }
    
};

#endif // UPDATEMANAGEMENT_H
