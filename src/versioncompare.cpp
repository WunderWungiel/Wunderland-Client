// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#include "versioncompare.h"
#include <QStringList>

// The number at the start of part (0 if it doesn't start with a digit)
static int leadingNumber(const QString &part) {
    int end = 0;
    while (end < part.length() && part.at(end).isDigit()) ++end;
    return part.left(end).toInt();
}

int compareVersions(const QString &a, const QString &b) {
    QStringList partsA = a.split('.');
    QStringList partsB = b.split('.');
    int count = qMax(partsA.count(), partsB.count());

    for (int i = 0; i < count; ++i) {
        int numberA = i < partsA.count() ? leadingNumber(partsA.at(i)) : 0;
        int numberB = i < partsB.count() ? leadingNumber(partsB.at(i)) : 0;
        if (numberA < numberB) return -1;
        if (numberA > numberB) return 1;
    }
    return 0;
}

bool isNewer(const QString &installed, const QString &available) {
    if (installed.isEmpty() || available.isEmpty()) return false;
    return compareVersions(available, installed) > 0;
}
