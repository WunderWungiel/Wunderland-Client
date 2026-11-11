# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Wunderland is a native Symbian (Anna / Belle) client for the [Wunderland Store](https://github.com/WunderWungiel/Wunderland-Store). It is a rebranded fork of schumi1331's AppList client, so comments and some identifiers still say "AppList". Stack: Qt 4.7/4.8 + QtQuick 1.1 + Symbian Qt Components (`com.nokia.symbian 1.1`) + Qt Mobility (feedback, systeminfo, publishsubscribe), plus native Symbian C++ APIs.

## Building

There is no test suite, linter, or desktop build. The project only builds with the Symbian Qt SDK (Qt 4.7.4, `C:\QtSDK\Symbian\SDKs\Symbian3Qt474`). Build with `build_symbian.bat [release|debug]` (qmake → `make release-gcce` → `make sis`, in-source); it produces an unsigned `Wunderland.sis`. Generated files (`Makefile`, `*.mmp`, `*.pkg`, `bld.inf`, `moc_*.cpp`, `qrc_*.cpp`) are gitignored.

- `HSWidgetxWunderland*.dll` are prebuilt homescreen-widget publisher binaries; they are not built from this repo. The `DEPLOYMENT` rules in the `.pro` package `C:\HSWidgetxWunderland_E9320736_rel.dll` and `C:\Wunderland.png` from the drive root, so both must exist there or `make sis` fails.
- Debug output (`qDebug`/`qWarning`) goes through `Logger` (`logger.*`, ported from Symbisend) to `download/Wunderland/log.txt` on F:, E: or C:/Data (truncated on each start).
- `#xqinstaller*` sources are intentionally commented out of `SOURCES`/`HEADERS`.

## Architecture

- `main.cpp` owns the one `QNetworkAccessManager` and the `StoreApi` built on it, and constructs the C++ backend singletons and exposes them to QML as context properties: `getList`, `appManagement`, `news`, `update`, `l` (Language), `subscriber`, `settings`, `symbianFunctions`, `logger`, plus `launchAppId` (parsed from an `appListId:<n>` launch argument). It also registers the homescreen widget (`HSWidget`) before the UI starts.
- QML entry point is `qrc:/qml/Wunderland/MainAppLoader.qml` → `main.qml` (PageStackWindow; pages pushed with `Qt.createComponent`). **All QML, images and `.qm` files are loaded from `Qml.qrc`, not from disk** (`DEPLOYMENTFOLDERS` is disabled) — any new QML/image/translation file must be added to `Qml.qrc`.
- `StoreApi` (`storeapi.*`) is the only way to talk to the server: base URL (default `http://ovi.wunderwungiel.pl`, overridable via the `network/baseUrl` setting), `buildUrl()` adding the common `os`/`resolution`/`language` parameters, `get()` returning an `ApiReply` that follows redirects, times out after 30 s without data, checks errors/HTTP status, emits `finished()` once and deletes itself. Errors are `StoreApi::Error` codes. `download()` is the same but streams the body into a `QIODevice` (used by `Downloader`, which saves to `download/Wunderland` on F:, E: or C:/Data). `GetList`, `UpdateManagement`, `News`, `Update`, `HSWidget` and `Downloader` take a `StoreApi*`.
- `GetList` is the central model: it fetches app lists from `applist.php` (latest / category / search / personal / updates), parses the XML response with `AppListParser` (`applistparser.*`, `QXmlStreamReader`; also used by `UpdateManagement`, `News` and `Update`) into `AppEntry`s, and appends them to five `AppListModel`s (`applistmodel.*`, a `QAbstractListModel` over `QList<AppEntry>`; QML reads a row with `get(row)`). `appentry.h` holds the `AppEntry` struct, the `AppRole` enum and `appRoleNames()` (the names QML reads); use the enum, never `Qt::UserRole + N` literals, and keep the role numbers stable. It also drives download/install via `Downloader` and `AppManagement`.
- `AppManagement` wraps Symbian's SIS registry (`sisregistrysession.h`/`sisregistryentry.h`, `-lsisregistryclient`) to detect installed apps and compare versions. On first run it maps installed UIDs to store IDs using the bundled `apps.txt`.
- `UpdateManagement` checks for updates of installed apps; it is used both by `GetList` and by `HSWidget` (homescreen widget update count). `Update` checks for a new version of Wunderland itself (`version.xml` / `changelog.xml` on the server). `News` reads `feed.xml`.
- Only the default (self-signed) variant of an app is supported; the AppList unsigned/Store variants (`uidstore`, `versionunsigned`, ...) were removed. Their old `Qt::UserRole` numbers are left as gaps. The `installed` role is `"<uid>1"` when installed and `"0"` when not.
- Persistent state uses `QSettings("WunderWungiel", "Wunderland")` (keys like `settings/languages`, `apps/installedIds`, `apps/highestSeenId`).
- `resources/*.xml` are sample/template server files, not used at runtime.

## Translations

- `.ts` sources and compiled `.qm` files are both committed at the repo root; `wunderland_template.ts` is the base for new languages.
- Adding a language requires: the `.ts`/`.qm` pair, an entry in `TRANSLATIONS` in `Wunderland.pro`, the `.qm` in `Qml.qrc`, and the language code (alphabetical) in `_languageCodes` in `language.cpp` (with its display name in `_allLanguages` at the same index). The UI translator is loaded from `:/wunderland_<code>` based on the device language.
- Category names are translated in `Language::initCategories()`; their order defines category IDs (head categories are 0, 12, 19).

## Commits

Don't put refactoring-plan step numbers (e.g. "plan 2.1") in commit titles. Describe the change itself.

## Version bumps

The version is hardcoded in three places that must be updated together: `VERSION` in `Wunderland.pro` (e.g. `2.0.5`), and the display strings in `qml/Wunderland/AboutPage.qml` and `qml/Wunderland/SplashScreen.qml` (e.g. `2.00(5)`).
