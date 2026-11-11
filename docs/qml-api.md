# Wunderland QML API

The C++ objects the QML in `qml/Wunderland/` can use. All of them are context properties
set in `main.cpp`, except the `InstallManager`, `AppCatalog` and `StoreApi` enums (QML import
`Wunderland 1.0`; the types are not creatable). Keep this
file up to date when the QML-facing API changes.

QtQuick 1.1: invokables return plain values (enums as `int`), and a list row is read as a
JS object with `get(row)`.

Names starting with `_` (`appCatalog._mainList`, `news._model`) and the getter-style
invokables are AppList leftovers, to be renamed in plan step 8.4.

## `appCatalog` — the store app lists

Five fixed `AppListModel`s (see [App list models](#app-list-models)), all loaded from
`applist.php`:

| property                    | filled by                        | used on                         |
|-----------------------------|----------------------------------|---------------------------------|
| `_mainList`                 | `loadMainList()`                 | main page (the 7 newest apps)   |
| `_categoryList`             | `loadCategoryList(...)`          | category page                   |
| `_searchList`               | `loadSearchList(...)`            | search page                     |
| `_personalList`             | `loadPersonalList(...)`          | "My apps" (installed, newest install first) |
| `_updateList`               | `loadUpdateList(...)`            | updates page, and opening one app by id |

The model objects never change (`CONSTANT`); a finished load changes their rows.

Methods. `start` is the row offset for paging (the server sends 25 rows per page), `clean`
clears the list when the load finishes (true for page 1, false to append the next page):

- `loadMainList()` — always clears.
- `loadCategoryList(categoryId, start, clean)` — `categoryId` as in `l.categories()`.
- `loadSearchList(searchTerm, start, clean)` — the term is URL-encoded here.
  `getLastSearchTerm()` returns the last term passed (for the next page).
- `loadPersonalList(start, clean)` — the apps in `updateTracker`'s installed list. Apps
  that are no longer installed are left out (and dropped from that list).
- `loadUpdateList(start, clean)` — the apps in `updateTracker`'s update list.
- `loadUpdateList(id)` — only the app with this store id, into `_updateList` (cleared).
  `main.qml` uses it for `launchAppId` and `subscriber`, then opens
  `_updateList.get(0)` on `updateListChanged`.
- `retry(listId)` — repeats the last request of that list (same rows, same `clean`), after
  a `loadingError`. Does nothing if the list was never loaded.

Every request sends the device's `os` and `resolution`; the main, category and search
lists also send the store languages selected in `l` (the personal and update lists are
by id, so they don't filter by language).

Signals:
- `mainListChanged()`, `categoryListChanged()`, `searchListChanged()`,
  `personalListChanged()`, `updateListChanged()` — a load of that list finished, also when
  it added no rows. Not emitted on an error.
- `loadingError(listId, error)` — a list could not be loaded. `listId`: 0 main,
  1 category, 2 search, 3 personal, 4 update (`AppCatalog.MainList` ... `AppCatalog.UpdateList`).
  `error`: a [`StoreApi` error](#error-codes). The list keeps its rows. `AppListView` with
  that `listId` stops its spinner and shows a banner that calls `retry()` when tapped;
  `main.qml` does the same for an app it was asked to open.
- `appListUpdate()` — the server requires a newer Wunderland (`minversion` in the
  response); the list is not changed. `main.qml` opens `UpdatePage.qml`.

When `installManager` reports an installed app (`appInstalled`), its rows in every list
change to `installed` = true, `updateAvailable` = false.

## App list models

`AppListModel` (`applistmodel.*`): a row is one app (`AppEntry`, `appentry.h`).

Properties: `count` (int). Methods: `get(row)` → object with every role below by name,
`{}` if `row` is out of range. `DetailPage.qml` keeps such an object as `detailItem`; it is
a snapshot and does not follow later changes of the row.

Roles (`model.<role>` or the bare name in delegates; `get(row)["<role>"]`). All come from
the server unless noted:

| role              | type     | notes                                                    |
|-------------------|----------|----------------------------------------------------------|
| `id`              | int      | store id                                                 |
| `name`            | string   |                                                          |
| `uid`             | string   | Symbian UID as hex text, e.g. `0x200622D0`; used for install checks  |
| `icon`            | string   | URL                                                      |
| `version`         | string   | e.g. `1.2.3`                                             |
| `versiondate`     | datetime | invalid if the server sent none                          |
| `installed`       | bool     | client: installed (any version), from the SIS registry   |
| `updateAvailable` | bool     | client: installed in an older version than `version`     |
| `category`        | int      | category id, see `l.categoryName()`                      |
| `language`        | string   |                                                          |
| `os`              | string   |                                                          |
| `developer`       | string   |                                                          |
| `mail`            | string   | may be empty (as may all the links below)               |
| `website`         | string   |                                                          |
| `twitter`         | string   |                                                          |
| `facebook`        | string   |                                                          |
| `donation`        | string   |                                                          |
| `aas`             | string   |                                                          |
| `price`           | double   |                                                          |
| `description`     | string   |                                                          |
| `image1`…`image5` | string   | screenshot URLs, empty if none                           |
| `tags`            | string   |                                                          |
| `changelog`       | string   |                                                          |
| `download`        | string   | SIS URL, or a file name under `<baseUrl>/static/content/files/` |
| `downloadsize`    | string   | bytes as text (`"0"` if unknown)                         |

The role numbers are fixed (`AppRole` in `appentry.h`); only the names matter to QML.

## `installManager` — download and install, one app at a time

Enums (`import Wunderland 1.0`, then `InstallManager.<name>`; not creatable):
- `Action`: `Download` (0, not installed), `Update` (1, an older version is installed),
  `Launch` (2, installed and up to date), `Downloading` (3, this app is being downloaded
  or installed now).
- `State`: `Idle` (0), `Transferring` (1, the SIS is downloading), `Installing` (2, the
  installer is open).

Properties: `state` (a `State`), `progress` (int, download percent).

Methods (`app` is a row from `get(row)`):
- `buttonAction(app)` → `Action` for the detail page button. Reads the SIS registry, not
  the `installed` role, so it is right even if the snapshot is old. Call it again after
  `installServerFinished`/`downloadError`.
- `buttonPressed(app, action)` — `Download`/`Update` start the download (and a statistics
  request); `Launch` starts the app. Returns false only if the download failed to start
  (then `downloadError` was already emitted); true means show the progress.
  Ignored while another download runs.
- `cancelDownload()` → false if nothing was downloading. Cannot cancel the installer.

Signals:
- `stateChanged()`, `progressChanged()`
- `downloadError(code)` — back to `Idle`. `code`: [`Downloader` error](#error-codes).
- `installServerFinished(statusCode)` — the installer closed. `0`: the downloaded version
  is installed now; `1`: failed or cancelled. The SIS file is deleted either way.
- `appInstalled(id)` — after a successful install (also sent to `appCatalog`, see above).

## `updateTracker` — installed apps with updates

- `count` (int) — number of installed store apps with a newer version on the server.
  Badges on the main page and the toolbar bind to it.
- Signal `countChanged()`.

The check itself runs from C++ (at startup, and from the homescreen widget once a day);
QML only reads the result. The update ids it keeps are what `loadUpdateList(start, clean)`
loads.

## `l` — languages and categories (`Language`)

Store languages (the languages of apps the user wants to see; persisted):
- `languageCodes` (string list) — every supported code (`bg`, `cs`, …, `zh`), sorted.
- `getLanguageName(index)` → display name for `languageCodes[index]` (in that language).
- `getSelected(code)` → whether the user selected it.
- `addLanguage(code)`, `removeLanguage(code)` — change the selection.
- `subtitleString` (string) — the selection for the settings row, e.g. `EN, PL`.
- Signal `myLanguagesChanged()` (the NOTIFY of both properties).

The UI language is the phone's language (if a translation exists, else English) and
cannot be changed at runtime.

Categories (names translated):
- `categories()` → list of names; the index is the category id.
- `categoryName(id)` → name, `""` for an unknown id.
- `isHeadCategory(id)` → true for 0 (Applications), 12 (Games), 19 (Themes). The
  subcategories of a head category are the ids up to the next head category.

## `settings` — user options (persisted)

All read/write bool; assign directly (`settings.darkTheme = checked`).

- `darkTheme` (default true) — read by `main.qml` at startup only: applies after a restart.
  `main.qml` turns it into colour roles on `window` (`bgColor`, `textColor`, `subtleColor`,
  `lineColor`, `highlightColor`, `bgTransparentColor`) and `window.darkTheme`; pages use those.
- `aasFeed` (default false) — show the news feed on the main page; also at startup only.
- `newsOpenInBrowser` (default false) — news items open in the default browser instead
  of inside the app.

Signals: `darkThemeChanged()`, `aasFeedChanged()`, `newsOpenInBrowserChanged()`.

## `news` — the store news feed (`feed.xml`)

- `_model` — list model, roles `title`, `description` (plain text, HTML stripped),
  `date` (string, the feed's `pubDate` unchanged), `link` (string).
- `fetch()` — loads the feed and replaces the rows. Ignored while a fetch is running
  (`updateRunning()` → bool).
- Signal `listChanged()` — a fetch finished. On an error the old rows are kept.
- `fetchHtml(link)` → signal `htmlChanged()`, then `getHtml()` returns the page (or a short
  error page).
- `feedUrl()` → the feed URL. `setFeedUrl(url)` does nothing (plan 7.6).

## `update` — new versions of Wunderland itself

- `loadUpdateData(all)` — `all` false: reads `version.xml`, then signal
  `loadingDone(loadResult, message)` (`loadResult` false if it could not be loaded;
  `message` describes the new version). `all` true: reads `changelog.xml` into
  `changelog`.
- `changelog` (string) — with signal `changelogChanged()` (also emitted when loading
  failed; then the old text stays).
- `startDownload()` — downloads the SIS from `version.xml`'s `url` and opens the
  installer. Signals `progressChanged(progress)` (percent) and `downloadError(code)`
  ([`Downloader` error](#error-codes)).

## `subscriber` — requests from other apps

- Signal `appListIdReceived(appListId)` — another app (or the homescreen widget) asked,
  through the Qt Mobility value space path `/appList` (value `appListId:<n>`), to show
  the app with store id `n`. Never emitted on desktop builds.

The path and the value format are kept from AppList on purpose: other apps and the widget
DLL use them.

## `launchAppId` — int

The store id from an `appListId:<n>` command line argument (Wunderland started by another
app to show that app), else `-1`.

## `symbianFunctions`

- `openUrlInDefaultBrowser(url)` → bool — opens `url` in the browser chosen in the phone
  settings (unlike `Qt.openUrlExternally`, which always uses the built-in one). False if no
  browser could be started. On desktop it uses `QDesktopServices`.

## `logger`

Not used by the QML yet; for a debug/log page.
- `logText` (string) — the recent log lines joined with `\n`; signal `logChanged()`.
- `model` — list model of the recent lines, role `line`.
- `logFilePath` (string) — the log file (`…\download\Wunderland\log.txt` on the device).
- `clear()` — clears the in-memory lines.

## `appManagement`

Exposed, but not used by the QML. `checkAppInstalled(uid)` → bool. Prefer
`installManager.buttonAction()` for install state on pages.

## Error codes

`StoreApi::Error` (`appCatalog.loadingError`; in QML `StoreApi.NetworkError`, ...;
`window.loadingErrorText(error)` gives the text):

| code | name               | meaning                                         |
|------|--------------------|-------------------------------------------------|
| 0    | `NoError`          |                                                 |
| 1    | `NetworkError`     | no connection, host not found, refused, ...     |
| 2    | `TimeoutError`     | no data for 30 s                                |
| 3    | `HttpError`        | the server answered with status >= 400          |
| 4    | `TooManyRedirects` |                                                 |
| 5    | `InvalidResponse`  | empty or not the expected XML                   |
| 6    | `WriteError`       | (downloads only)                                |

`Downloader::ErrorCode` (`installManager.downloadError`, `update.downloadError`;
`window.downloadErrorText(code)` gives the text for all but 1):

| code | name              | meaning                                                     |
|------|-------------------|-------------------------------------------------------------|
| 0    | `ConnectionError` | network or HTTP error, timeout, too many redirects          |
| 1    | `NoSpaceError`    | no drive with enough free space                             |
| 2    | `WriteError`      | the file could not be created or written                    |
| 3    | `InstallerError`  | downloaded, but the installer could not be started          |
