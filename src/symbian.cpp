#include "symbian.h"
#include "platform/apps.h"

Symbian::Symbian(QObject *parent) :
    QObject(parent)
{
}

bool Symbian::openUrlInDefaultBrowser(QString url) {
    return Platform::openUrlInBrowser(url);
}
