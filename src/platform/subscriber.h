// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <QObject>

#ifdef Q_OS_SYMBIAN
#include <qmobilityglobal.h>

QTM_BEGIN_NAMESPACE
class QValueSpaceSubscriber;
QTM_END_NAMESPACE
#endif

// Emits appListIdReceived() when another app asks (through the Qt Mobility
// value space, path /appList) to show an app. Never emits on other platforms,
// which have no Qt Mobility. The path and the appListId:<n> value are kept
// from AppList on purpose: other apps and the widget DLL use them.
class Subscriber : public QObject
{
    Q_OBJECT
public:
    explicit Subscriber(QObject *parent = 0);
    
signals:
    void appListIdReceived(int appListId);
    
public slots:
    void getValueChanged();

private:
#ifdef Q_OS_SYMBIAN
    QtMobility::QValueSpaceSubscriber *qsubscriber;
#endif
};

#endif // SUBSCRIBER_H
