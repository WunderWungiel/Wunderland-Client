// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symbian.h"
#include "platform/apps.h"

Symbian::Symbian(QObject *parent) :
    QObject(parent)
{
}

bool Symbian::openUrlInDefaultBrowser(QString url) {
    return Platform::openUrlInBrowser(url);
}
