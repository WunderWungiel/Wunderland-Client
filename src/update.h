// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

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
    void downloadError(int code); // a Downloader::ErrorCode

    void changelogChanged();

public slots:
    void parseData();

private:
    StoreApi *api;
    QString fileUrl;

    QString _changelog;

    Downloader *downloader;
};

#endif // UPDATE_H
