#include "settings.h"

Settings::Settings(QObject *parent) :
    QObject(parent)
{
}

bool Settings::darkTheme() {
    QSettings settings("WunderWungiel", "Wunderland");
    return settings.value("settings/darkTheme", true).toBool();
}

void Settings::setDarkTheme(bool newValue) {
    QSettings settings("WunderWungiel", "Wunderland");
    settings.setValue("settings/darkTheme", newValue);
}

bool Settings::aasFeed() {
    QSettings settings("WunderWungiel", "Wunderland");
    return settings.value("settings/aasFeed", false).toBool();
}

void Settings::setAasFeed(bool newValue) {
    QSettings settings("WunderWungiel", "Wunderland");
    settings.setValue("settings/aasFeed", newValue);
}
