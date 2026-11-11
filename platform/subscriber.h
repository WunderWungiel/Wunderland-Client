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
// which have no Qt Mobility.
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
