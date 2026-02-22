#include "language.h"

Language::Language(QObject *parent) :
    QObject(parent)
{
    QtMobility::QSystemInfo si;
    _currentLanguage = si.currentLanguage().toLower();

    _allLanguages << QString::fromUtf8("\u0411\u044A\u043B\u0433\u0430\u0440\u0441\u043A\u0438") << QString::fromUtf8("\u010Ce\u0161tina") << "Dansk" << "Deutsch" << QString::fromUtf8("\u0395\u03bb\u03bb\u03b7\u03bd\u03b9\u03ba\u03ac") << "English" << "Espa�ol" << "Suomi" << "Fran�ais" << "Magyar" << "Italiano" << "Nederlands" << "Polski" << "Portugu�s" << QString::fromUtf8("\u0420\u0443\u0441\u0441\u043A\u0438\u0439") << "Svenska" << "T�rk�e" << QString::fromUtf8("\u0423\u043A\u0440\u0430\u0457\u043D\u0441\u044C\u043A\u0430") << QString::fromUtf8("\u4E2D\u6587");
    _languageCodes << "bg" << "cs" << "da" << "de" << "el" << "en" << "es" << "fi" << "fr" << "hu" << "it" << "nl" << "pl" << "pt" << "ru" << "sv" << "tr" << "uk" << "zh";

    if (_currentLanguage != "en" && _languageCodes.contains(_currentLanguage)) {
        translator = new QTranslator(this);
        translator->load(("wunderland_"+_currentLanguage), ":/");
        QCoreApplication::instance()->installTranslator(translator);
    }

    QSettings settings("WunderWungiel", "Wunderland");
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

        QSettings settings("WunderWungiel", "Wunderland");
        settings.setValue("settings/languages", _myLanguages);

        emit myLanguagesChanged();
    }
}

void Language::removeLanguage(QString languageCode) {
    if (_myLanguages.contains(languageCode)) {
        _myLanguages.removeAll(languageCode);

        QSettings settings("WunderWungiel", "Wunderland");
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
