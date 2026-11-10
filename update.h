#ifndef UPDATE_H
#define UPDATE_H

#include <QObject>
#include <QDebug>
#include "downloader.h"
#include "storeapi.h"

class Update : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString changelog READ changelog NOTIFY changelogChanged)
public:
    explicit Update(StoreApi *api, QObject *parent = 0);

    Q_INVOKABLE void loadUpdateData(bool all = false);
    Q_INVOKABLE void startDownload();

    QString changelog() { return _changelog; }

signals:
    void loadingDone(bool loadResult, QString message);
    void progressChanged(int progress);

    void changelogChanged();

public slots:
    void parseData();

    void progressChangedReceived(int percent) {
        emit progressChanged(percent);
    }

private:
    StoreApi *api;
    QString fileUrl;

    QString _changelog;

    Downloader downloader;
};

#endif // UPDATE_H
