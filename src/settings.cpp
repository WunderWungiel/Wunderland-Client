// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings.h"

#include <QSettings>

// Existing keys, so that current users keep their options when they upgrade.
static const char *kDarkThemeKey = "settings/darkTheme";
static const char *kAasFeedKey = "settings/aasFeed";
static const char *kNewsOpenInBrowserKey = "news/openInBrowser";

Settings::Settings(QObject *parent) :
    QObject(parent)
{
    QSettings s;
    m_darkTheme = s.value(QLatin1String(kDarkThemeKey), true).toBool();
    m_aasFeed = s.value(QLatin1String(kAasFeedKey), false).toBool();
    m_newsOpenInBrowser = s.value(QLatin1String(kNewsOpenInBrowserKey), false).toBool();
}

bool Settings::store(const char *key, bool *member, bool value)
{
    if (*member == value)
        return false;
    *member = value;
    QSettings().setValue(QLatin1String(key), value);
    return true;
}

void Settings::setDarkTheme(bool on)
{
    if (store(kDarkThemeKey, &m_darkTheme, on))
        emit darkThemeChanged();
}

void Settings::setAasFeed(bool on)
{
    if (store(kAasFeedKey, &m_aasFeed, on))
        emit aasFeedChanged();
}

void Settings::setNewsOpenInBrowser(bool on)
{
    if (store(kNewsOpenInBrowserKey, &m_newsOpenInBrowser, on))
        emit newsOpenInBrowserChanged();
}
