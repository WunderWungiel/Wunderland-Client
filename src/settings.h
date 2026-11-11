// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>

// User options, persisted in QSettings (organization and application name
// are set once in main.cpp). Read once at construction; each setter writes
// through and emits its NOTIFY signal.
//
// darkTheme and aasFeed are read by main.qml at startup only, so changing
// them takes effect after a restart (SettingsPage.qml says so).
class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool darkTheme READ darkTheme WRITE setDarkTheme NOTIFY darkThemeChanged)
    // Show the Wunderland Store news feed on the main page.
    Q_PROPERTY(bool aasFeed READ aasFeed WRITE setAasFeed NOTIFY aasFeedChanged)
    // Open news items in the default browser instead of in the app.
    Q_PROPERTY(bool newsOpenInBrowser READ newsOpenInBrowser WRITE setNewsOpenInBrowser NOTIFY newsOpenInBrowserChanged)

public:
    explicit Settings(QObject *parent = 0);

    bool darkTheme() const { return m_darkTheme; }
    void setDarkTheme(bool on);

    bool aasFeed() const { return m_aasFeed; }
    void setAasFeed(bool on);

    bool newsOpenInBrowser() const { return m_newsOpenInBrowser; }
    void setNewsOpenInBrowser(bool on);

signals:
    void darkThemeChanged();
    void aasFeedChanged();
    void newsOpenInBrowserChanged();

private:
    // Stores value under key if it differs from *member; returns true if it did.
    static bool store(const char *key, bool *member, bool value);

    bool m_darkTheme;
    bool m_aasFeed;
    bool m_newsOpenInBrowser;
};

#endif // SETTINGS_H
