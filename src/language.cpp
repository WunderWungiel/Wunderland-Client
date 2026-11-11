// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#include "language.h"
#include "platform/device.h"

namespace {
struct LanguageEntry {
    const char *code;
    const char *name;   // UTF-8
};

// Code and display name in one table, so the two lists cannot drift apart.
// Keep sorted by code.
const LanguageEntry kLanguages[] = {
    { "bg", "\u0411\u044A\u043B\u0433\u0430\u0440\u0441\u043A\u0438" },
    { "cs", "\u010Ce\u0161tina" },
    { "da", "Dansk" },
    { "de", "Deutsch" },
    { "el", "\u0395\u03bb\u03bb\u03b7\u03bd\u03b9\u03ba\u03ac" },
    { "en", "English" },
    { "es", "Espa\u00F1ol" },
    { "fi", "Suomi" },
    { "fr", "Fran\u00E7ais" },
    { "hu", "Magyar" },
    { "it", "Italiano" },
    { "nl", "Nederlands" },
    { "pl", "Polski" },
    { "pt", "Portugu\u00EAs" },
    { "ru", "\u0420\u0443\u0441\u0441\u043A\u0438\u0439" },
    { "sv", "Svenska" },
    { "tr", "T\u00FCrk\u00E7e" },
    { "uk", "\u0423\u043A\u0440\u0430\u0457\u043D\u0441\u044C\u043A\u0430" },
    { "vi", "Ti\u1EBFng Vi\u1EC7t" },
    { "zh", "\u4E2D\u6587" },
};
}

Language::Language(QObject *parent) :
    QObject(parent)
{
    _currentLanguage = Platform::deviceLanguage();

    for (unsigned i = 0; i < sizeof(kLanguages) / sizeof(kLanguages[0]); ++i) {
        _languageCodes << QString::fromLatin1(kLanguages[i].code);
        _allLanguages << QString::fromUtf8(kLanguages[i].name);
    }

    if (_currentLanguage != "en" && _languageCodes.contains(_currentLanguage)) {
        translator = new QTranslator(this);
        translator->load(("wunderland_"+_currentLanguage), ":/");
        QCoreApplication::instance()->installTranslator(translator);
    }

    QSettings settings;
    if (!settings.contains("settings/languages")) {
        // Init
        _myLanguages << "en";
        if (_currentLanguage != "en") _myLanguages << _currentLanguage;
        _myLanguages.sort();
        settings.setValue("settings/languages", _myLanguages);
    } else {
        _myLanguages = settings.value("settings/languages").toStringList();
    }

    initCategories();
}

void Language::addLanguage(QString languageCode) {
    if (!_myLanguages.contains(languageCode)) {
        _myLanguages.append(languageCode);

        _myLanguages.sort();

        QSettings settings;
        settings.setValue("settings/languages", _myLanguages);

        emit myLanguagesChanged();
    }
}

void Language::removeLanguage(QString languageCode) {
    if (_myLanguages.contains(languageCode)) {
        _myLanguages.removeAll(languageCode);

        QSettings settings;
        settings.setValue("settings/languages", _myLanguages);

        emit myLanguagesChanged();
    }
}

QString Language::subtitleString() {
    QString ret;
    for (int i=0; i<_myLanguages.count(); i++) {
        ret += _myLanguages.at(i).toUpper();
        if (i+1<_myLanguages.count()) ret += ", ";
    }
    return ret;
}

QString Language::languagesForUrl(){
    QString ret = _currentLanguage.toUpper();
    for (int i=0; i<_myLanguages.count(); i++) {
        if (QString::compare(_myLanguages.at(i), _currentLanguage) == 0) continue;
        ret += "," + _myLanguages.at(i).toUpper();
    }
    return ret;
}

/* Categories */
bool Language::isHeadCategory(int __id) {
    // Our head category ids are 0 (Applications), 12 (Games) and 19 (Themes)
    if (__id == 0 || __id == 12 || __id == 19) return true;
    else return false;
}

QString Language::categoryName(int __id) {
    if (__id > -1 && __id < _categories.count()) return _categories.at(__id);
    else return "";
}

void Language::initCategories() {
    _categories.append(tr("Applications"));
    _categories.append(tr("Camera, photos, videos"));
    _categories.append(tr("Emulator"));
    _categories.append(tr("Extras"));
    _categories.append(tr("File manager & cloud"));
    _categories.append(tr("Internet"));
    _categories.append(tr("Music"));
    _categories.append(tr("Office"));
    _categories.append(tr("Other apps"));
    _categories.append(tr("Social"));
    _categories.append(tr("Tools"));
    _categories.append(tr("Weather & GPS"));
    _categories.append(tr("Games"));
    _categories.append(tr("Action"));
    _categories.append(tr("Adventure"));
    _categories.append(tr("Other games"));
    _categories.append(tr("Puzzles & cards"));
    _categories.append(tr("Sports"));
    _categories.append(tr("Strategy"));
    _categories.append(tr("Themes"));
}
