// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#include "subscriber.h"
#include <QDebug>
#include <QStringList>

#ifdef Q_OS_SYMBIAN
#include <qvaluespacesubscriber.h>

QTM_USE_NAMESPACE
#endif

Subscriber::Subscriber(QObject *parent) :
    QObject(parent)
{
#ifdef Q_OS_SYMBIAN
    qsubscriber = new QValueSpaceSubscriber("/appList", this);
    connect(qsubscriber, SIGNAL(contentsChanged()), this, SLOT(getValueChanged()));
#endif
}

void Subscriber::getValueChanged() {
#ifdef Q_OS_SYMBIAN
    qDebug() << "subscriber";

    const QStringList subPaths = qsubscriber->subPaths();
    for (int i = 0; i < subPaths.count(); ++i) {
        QString value = qsubscriber->value(subPaths.at(i)).toString();
        if (value.startsWith("appListId:")) {
            emit appListIdReceived(value.mid(10).toInt());
            break;
        }
    }
#endif
}
