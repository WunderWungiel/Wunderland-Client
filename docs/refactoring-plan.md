# Wunderland Client — Refactoring Plan

The steps are in priority order. Each one should be a single commit (or a small series) that
builds and gets checked on a device before the next one starts. There is no test suite, so
**Phase 0 comes first**: without a repeatable build and a log file, there is no way to check
the later steps.

Reference project: `~/Documents/Symbisend` (same author, same toolchain). It already solves
several of these problems: `build_symbian.bat`, `src/logger.*`, `src/settings.*` with
`Q_PROPERTY`, a `src/` layout, and `docs/qml-api.md` as the C++↔QML contract. Copy its
patterns instead of inventing new ones.

Constraints (same as Symbisend): Qt 4.7.4 SDK compiling for a Qt 4.8 runtime, pre-C++11 (no
`auto`, lambdas, `nullptr` or new-style `connect`), QtQuick 1.1.

---

## [x] Phase 0 — Make changes verifiable (done)

### [x] 0.1 Add `build_symbian.bat`
Port Symbisend's script (qmake → `make release-gcce` → `make sis`). Decide whether Wunderland
ships self-signed or unsigned (`QT_SIS_OPTIONS=-o`). Every later step must produce a `.sis`
from this script.

### [x] 0.2 Bring back logging
`main.cpp` has a commented-out `SimpleLoggingHandler` that writes to `c:\data\applistlog.txt`
through `std::ofstream`. Replace it with Symbisend's `Logger` (Qt message handler, file on
`F:/E:/C:/Data`, `Component:` prefixes). Without it, nothing can be debugged on a device.
Optionally add a hidden Log page later.

### [x] 0.3 Remove `QT += testlib` from `Wunderland.pro`
Nothing uses QTest. It only makes the binary bigger and adds a runtime dependency.

---

## [ ] Phase 1 — Fix real bugs (small, low-risk, high value)

These are bugs I confirmed by reading the code. Fix them before any restructuring, so that
the refactor doesn't hide them or change their behaviour by accident.

### [x] 1.1 Language list off by one (regression from `e4d2284`) — **crash**
`language.cpp:9-10`: *(Fix exists on the unmerged branch `fix-vietnamese-language-name`.)* `"vi"` was added to `_languageCodes` (now 20 entries), but no display
name was added to `_allLanguages` (still 19). `MultiSelectionDialog.qml` calls
`l.getLanguageName(index)`, so Vietnamese shows as "中文", and `zh` (index 19) runs
`QList::at()` past the end. Add `"Tiếng Việt"` before Chinese. Better: replace the two
parallel lists with one table of `{code, name}` pairs so they can't drift apart again (see 3.4).

### [x] 1.2 Lost non-ASCII characters in sources
The files were converted from Latin-1 to UTF-8 at some point, and the umlauts/accents became
U+FFFD (`�`) for good:
- `language.cpp:9`: `"Espa�ol"`, `"Fran�ais"`, `"Portugu�s"`, `"T�rk�e"` are shown to users
  as broken text. Use `\u` escapes like the other entries.
- `news.cpp`: the `&#196;` → `�` replacements now replace entities with garbage. Delete them;
  `QTextDocument::setHtml` already decodes entities.
- `getlist.cpp`, `getlist.h`, `appmanagement.cpp`: German comments with `�`. Translate them to
  English (or remove them) while you're there.

### [x] 1.3 Signal connections pile up on every download — **duplicate install handling**
- `GetList::downloadDoneReceived()` (`getlist.h:174`) connects
  `appManagement.installServerFinished` → `installServerFinishedReceived` every time a download
  finishes.
- `AppManagement::startInstallServerListening()` (`appmanagement.cpp:151`) connects
  `installServerThread.finished` again every time.

After N downloads, one install runs the handler about N² times. The second and later calls see
`downloadingItem` already reset, so they emit `installServerFinished(1)` ("Installation
failed") for an install that worked. Fix: connect once in the constructors, or use
`Qt::UniqueConnection`.

### [x] 1.4 Redirect handling is broken in all four copies
`QUrl urlRedirectedTo = redirectUrl(possibleRedirectUrl.toUrl(), urlRedirectedTo);` passes an
object that isn't initialised yet into its own initialiser (`getlist.cpp:235`,
`updatemanagement.cpp`, `news.cpp`, `downloader.cpp:107`). Also:
- `GetList`: the follow-up request doesn't copy `objectName` (the list id), so a redirected
  category/search/personal list is parsed into list 0 (the main list).
- `Downloader::downloadFinished`: the redirect branch is commented out. A redirected download
  leaves `downloading == true` forever and the file open, and every later download is ignored
  until restart.
Fixed for good by the shared network layer in 2.1. A quick fix is fine first.

### [x] 1.5 Network errors are ignored, and replies leak
None of the `finished()` handlers check `reply->error()` or call `deleteLater()`. An HTTP 500
error page gets "parsed", and every request leaks a `QNetworkReply`. `GetList::loadingError`
is emitted with a hard-coded English string, and **no QML connects to it**, so the user sees an
endless spinner. Add `deleteLater()` everywhere now. Error handling comes with 2.1.

### [ ] 1.6 Search pagination always asks for `start=25`
`SearchPage.qml:118`: `getList.loadSearchList(searchField.text, 25, false)`. The third page
repeats the second one. Use `appList.count`, as `CategoryListPage`/`PersonalPage` do.

### [ ] 1.7 Search term is not URL-encoded
`getlist.cpp:178` pastes `searchTerm` into the query string. `&`, `#`, `+` or non-ASCII
characters break the request. Use `QUrl::addQueryItem` (Qt 4) instead of joining strings.

### [ ] 1.8 Undefined behaviour and small crashes
- `AppManagement::checkAppInstalled(QString, bool)` (`appmanagement.cpp:142`) returns nothing
  when `withSource == false`.
- `Downloader::progress` divides by `bytesTotal`, which is `-1`/`0` when the server sends no
  `Content-Length`.
- `InstallServerThread::run`: `"situationFlag: " + situationFlag` adds a bool to a pointer
  (`const char*`), so it prints garbage or reads out of bounds.
- `Downloader::startDownloadWithoutStats` (self-update) writes to a hard-coded `e:/installs/`
  without creating the folder or checking `file.open()`. On a phone with no memory card the
  self-update fails silently.
- `AppManagement::getVersion` ignores a failed `iSisRegSession.Connect()` and calls `VersionL()`
  three times.

### [ ] 1.9 User-facing "AppList" strings and placeholders
`main.qml:133`, `DetailPage.qml:544`, `SelectVersion.qml:41` still say "AppList".
`UpdatePage.qml:130` asks the user to "write a mail to xxx". `DetailPage.qml:385` writes
mail subjects as "… on AppList". The widget strings in `hswidget.cpp` ("updates available",
"new apps") are not translated. (Changing `qsTr` source strings invalidates the existing
translations. Do it in one batch together with a `lupdate` run.)

---

## [ ] Phase 2 — One network layer

Right now there are five `QNetworkAccessManager`s (GetList, UpdateManagement ×2, News, Update,
plus one per Downloader), four copies of the redirect helper, and the base URL
`http://ovi.wunderwungiel.pl` written out in about 10 places.

### [ ] 2.1 Add `StoreApi` (or `ApiClient`)
- One shared `QNetworkAccessManager`, owned in `main.cpp` and passed to whoever needs it.
- One `static const` base URL (or a value in QSettings, so a test server can be used).
- `QUrl buildUrl(const QString &path, const QList<QPair<QString,QString> > &query)`, which
  always adds `os`, `resolution`, `unsigned`, `language` (this code is copied in 6 `load*`
  methods today).
- A single place that handles a finished reply: follow redirects (with a hop limit), check
  `error()` and the HTTP status, decode UTF-8, call `deleteLater()`. Pass the request
  context with `QNetworkReply::setProperty("listId", …)` instead of `objectName`.
- Per-request timeouts with a `QTimer` (Qt 4.7 QNAM has none; Symbisend's `Sender` already
  does this). A dead connection currently spins forever.
- Emit errors as codes/enums. QML turns them into translated text.

### [ ] 2.2 Replace the hand-written XML parsing with `QXmlStreamReader`
`GetList::parseItem` and `UpdateManagement::parseItem` (identical copies) run `indexOf` over
the whole response, and the order matters: each field must come after the previous one, or
the parser reads the *next app's* value (and a missing field quietly shifts the rest).
`QXmlStreamReader` (QtCore, available in 4.7) parses each `<app>`
into a map, with no order dependency, and decodes entities properly. Do the same for
`News::parseFeed` (RSS) and `Update::parseData` (`version.xml`/`changelog.xml`). Write it
once as an `AppListParser` that returns `QList<AppEntry>` (see 3.1).

### [ ] 2.3 Move `Downloader` onto the shared layer
- Follow redirects (fixes 1.4 for downloads).
- Choose a download folder like Symbisend's `Settings::defaultDownloadFolder()` (F: > E: >
  C:/Data, in a real `…/Wunderland/` folder, not the drive root), and use the same logic for
  the self-update path.
- Check `file.open()`/`write()` results; on error or cancel, delete partial files.
- Make the stat ping (`applist-download.php`) fire-and-forget instead of a gate: today a
  failed stats request (`returnCode != "0"`) blocks the download.

---

## [ ] Phase 3 — Domain model: one source of truth for app data

### [ ] 3.1 Introduce an `AppEntry` struct and a role enum in one header
The 39 roles are defined as a local `struct ListEntry` inside the `GetList` constructor, but
used as bare magic numbers elsewhere (`Qt::UserRole + 13`, `+ 35` in `getlist.h`), and QML
reads them by name through `getModelData()`. Move them into `appentry.h`:
`enum AppRole { IdRole = Qt::UserRole + 1, … }` and a `roleNames()` function. Replace every
`Qt::UserRole + N` literal.

Delete the unused `AppItem`/`ReviewItem` classes (`appitem.*`, `reviewitem.*`: compiled but
never used), or turn `AppItem` into this struct if a `QObject` per app is really wanted.

### [ ] 3.2 Replace `RoleItemModel` (QStandardItemModel) with an `AppListModel`
A `QAbstractListModel` over `QList<AppEntry>` with `append(list)`, `clear()`,
`updateInstalledState(id)`, `get(row)` (for QML), and a `count` property. This is lighter on
memory than one `QStandardItem` × 39 `QVariant`s per app on a phone, and it allows per-row
`dataChanged` instead of `setItem` plus re-emitting the property's NOTIFY signal.

### [ ] 3.3 Put "which variant is installed / is there an update" in one place
The same 25-line block that decides between self-signed, unsigned and Store (with the special
case `uid == uidunsigned`) is copied in `GetList::parseList` (`getlist.cpp:~300`) and
`GetList::installServerFinishedReceived` (`getlist.h:~240`). Update detection is copied again
in `UpdateManagement::checkForUpdate` and once more in `GetList::buttonAction`. Make it one
pure function on `AppEntry`:
```cpp
enum Variant { NotInstalled, SelfSigned, Unsigned, Store };
struct InstallState { Variant variant; QString uid; QString version; };
InstallState resolveInstallState(const AppEntry &, const InstalledApps &);
bool hasUpdate(const AppEntry &, const InstallState &, bool useUnsigned);
```
Also remove the encoding `installed = uid + "1"` (a variant digit appended to the UID string
and split off again with `chop(1)` in 4 places), and the `"update"` placeholder UID that
`UpdateManagement::parseList` passes to mean "skip unsigned".

### [ ] 3.4 Version comparison
`AppManagement::compareVersions` parses with nested `indexOf`/`mid` and returns "is web newer",
under a name that doesn't say so. Replace it with `static int compareVersions(a, b)` (−1/0/1)
that splits on `.`, plus `isNewer(installed, available)`. Write a few input/output pairs in a
comment, or run them from a desktop `main()` (this is pure Qt, so it compiles without the
Symbian SDK).

### [ ] 3.5 Turn the `buttonAction` codes 0–7 into an enum
`GetList::buttonAction` returns magic numbers 0–7, and `buttonPressed` takes a different set
of magic numbers 0–4. Both are checked as literals in `DetailPage.qml` and
`SelectVersion.qml`. Expose a `Q_ENUMS(Action)` so QML can use `GetList.Launch` and similar
names.

---

## [ ] Phase 4 — Break up `GetList`

`GetList` (≈560 + 320 lines, much of the logic inside the header) does list fetching,
parsing, install-state tracking, the update count, download control, Store-redirect
handling through an application event filter, and QSettings access.

### [ ] 4.1 Remove the global "current list" state
QML calls `getList.setCurrentList(n)` on page activation, and `buttonAction(index)` then reads
`list[currentList]`. Opening a detail page from the update banner while another page is
active, or a `PageStatus` race, picks the wrong app. Pass the `AppEntry`/id (which
`DetailPage` already has as `detailItem`) to the action methods instead.

### [ ] 4.2 Split responsibilities
- `AppCatalog`: owns the 5 `AppListModel`s (main, category, search, personal, updates), and
  loads/paginates them through `StoreApi`.
- `InstallManager`: download → installer → wait for install → refresh state. It owns the
  `Downloader`, `InstallServerThread` and the `ApplicationActivate` event filter for the Store
  flow. Exposes `state`/`progress` as properties.
- `UpdateTracker`: `updateIds`, `installedIds`, `highestSeenId`, and the update count, shared
  by the app and `HSWidget` (today `HSWidget` and `GetList` each create their own
  `UpdateManagement`, which each create their own `AppManagement`).
Wire them together in `main.cpp`, like Symbisend does.

### [ ] 4.3 Exactly one `AppManagement`
There are currently four instances (`main.cpp:71`, `GetList`, and one inside each of the two
`UpdateManagement`s), and each has its own `InstallServerThread`. Make it a single object that
is passed in. Also give `Language` an owner (`main.cpp:76` creates it with `new` and never
frees it) and make `UpdateManagement` a child of its owner (`new UpdateManagement(false)` has
no parent).

### [ ] 4.4 Move logic out of headers
`getlist.h` has `installServerFinishedReceived()` (≈110 lines) and the event filter; `news.h`
has `fetch()`/`fetchHtml()`. Move them to `.cpp`. Put `Q_OBJECT` first in the class body
(`getlist.h` has it after the `Q_PROPERTY`s).

---

## [ ] Phase 5 — Symbian native code safety

### [ ] 5.1 Trap every leaving call
`getVersion`, `launchApp`, `checkAppInstalled`, `applications`, `openInstaller`,
`openUrlInDefaultBrowser` and `HSWidget::bringToFront` call `User::LeaveIfError`/`…L()`
functions and use `CleanupStack` straight from Qt slots and Q_INVOKABLEs. A leave there (for
example AppArc not being ready yet, or a corrupt SIS registry entry) is an uncaught exception
and the app crashes. Wrap each one in `TRAPD`/`QT_TRAP_THROWING` and turn the error into a
return value plus a log line.

### [ ] 5.2 Replace `InstallServerThread` polling
It sleeps 10 s and then lists all processes once per second on a separate thread to find out
when `installserver` exits. Options, best first: a `RProcess::Logon` on the installer process
started by `StartDocument`; a `QTimer` on the UI thread (no thread needed); or keep the poll,
but with a timeout, not as a `QThread` subclass, and only connected once (see 1.3).

### [ ] 5.3 Keep Symbian code behind one boundary
Collect the native calls (`AppManagement`'s SIS registry/AppArc, `Symbian`, the `HSWidget`
glue) into a `platform/` folder behind small interfaces, with `#ifdef Q_OS_SYMBIAN`, as
Symbisend does. Then the parser, models, version logic and network layer build on desktop Qt
4.8 for fast iteration and for quick checks of 3.4 and 2.2.

### [ ] 5.4 Clean up `HSWidget`
Move the widget registration/update count out of the hot startup path in `main.cpp`
(`registerWidget()` runs before the UI and shows up as splash-screen delay), translate its
strings, and delete the commented-out code (≈40 lines in `bringToFront`).

---

## [ ] Phase 6 — Settings and persistence

### [ ] 6.1 One `Settings` class with `Q_PROPERTY`s
`QSettings("WunderWungiel", "Wunderland")` is built from string literals in about 20 places,
in 6 classes. Settings live in `Settings` (`darkTheme`, `aasFeed`), `GetList`
(`apps/unsigned`, `settings/storeNote`), `News` (`news/openInBrowser`) and `Language`.
Follow Symbisend's `Settings`: call `QCoreApplication::setOrganizationName/ApplicationName`
once in `main.cpp` (so `QSettings()` needs no arguments), make every user option a
`Q_PROPERTY` with NOTIFY, and keep the key strings in one place. QML then binds
(`checked: settings.darkTheme`) instead of calling getter/setter pairs.

### [ ] 6.2 Keep app state apart from preferences
`apps/installedIds`, `apps/updateIds`, `apps/highestSeenId`, `settings/appsScanned` belong to
`UpdateTracker` (4.2), not to scattered code. Keep the existing keys, so that current users
keep their state when they upgrade.

---

## [ ] Phase 7 — QML layer

### [ ] 7.1 Write `docs/qml-api.md`
List every context property, role and invokable, as Symbisend does. Do it during Phases 3–6,
because those phases rename the QML-facing API (`_mainList` → `mainList`,
`getUseUnsigned()` → `settings.useUnsigned`, …).

### [ ] 7.2 Theme
`main.qml` defines colours as `white/black/grey` properties that mean the opposite in dark
mode (`white = "black"`). Rename them to roles (`bgColor`, `textColor`, `subtleColor`), or
use `platformInverted` consistently.

### [ ] 7.3 Push pages without `Qt.createComponent` strings everywhere
About 15 places call `pageStack.push(Qt.createComponent("X.qml"))`. Using the file name works
(`pageStack.push(Qt.resolvedUrl("X.qml"))`), or declare reused pages once as `Component`s.
`Qt.createComponent` without checking `status` hides load errors.

### [ ] 7.4 Simplify `DetailPage.qml` (563 lines)
Move the size/version formatting (`DetailPage.qml:104-105`, single-line nested ternaries)
into C++ (`AppEntry::displayVersion(useUnsigned)`, `displaySize(...)`) or a small JS helper.
Split the social-link buttons into a reusable `AppLink` (that file already exists).
Remove the `+ l.emptyString` retranslation hack once the language can't change at runtime,
or switch to a real `rev` property, as Symbisend's `i18n.rev` does.

### [ ] 7.5 Show errors
Connect `loadingError`/new error enums to an `InfoBanner` on the list pages, so a failed
request stops the spinner and offers a retry (see 1.5).

### [ ] 7.6 News
`News::setFeedUrl` is empty, `fetchHtml` loads arbitrary HTML into a `WebView`, and the
`formula1.com` rewrite is left over from another app. Decide whether news is still wanted;
if so, use the `QXmlStreamReader` RSS parser from 2.2.

---

## [ ] Phase 8 — Housekeeping

1. [ ] **Folder layout:** move C++ into `src/` (and `src/platform/`), as Symbisend does; update
   `.pro` (one entry per line, ` \` continuations, check with `grep -nF '\n'`).
2. [ ] **Dead code:** `xqinstaller*` (4 files, commented out of the `.pro`), `appitem.*`,
   `reviewitem.*`, `GetList(QObject*)` (empty constructor that leaves members uninitialised),
   `Language::emptyString` once 7.4 is done, commented-out blocks in `getlist.cpp`
   (`buttonAction`), `hswidget.cpp` and `downloader.cpp`. Clean up `SOURCES`/`HEADERS` in `.pro`:
   the commented-out `#xqinstaller*` lines in the middle of a ` \`-continued list (one with a
   ` \` and one without) are fragile. Delete them.
3. [ ] **One version source:** `VERSION` in `.pro` is already passed as `APP_Version`; expose
   `appVersion` as a context property and use it in `AboutPage.qml`/`SplashScreen.qml`
   instead of hard-coding it 3 times (CLAUDE.md "Version bumps").
4. [ ] **Rename leftovers:** QML import `com.applist.RoleItemModel` (goes away with 3.2), the
   `appListId:` launch-argument and `/appList` value-space path (*keep these*: other apps and
   the widget DLL may use them; document them instead).
5. [ ] **Licence headers** (SPDX), as Symbisend has, and a `CLAUDE.md` update after each phase.
6. [ ] **Line endings:** sources are CRLF; add `.gitattributes` so that edits don't create
   whole-file diffs.

---

## Suggested order and checkpoints

| # | Done | Steps | Checkpoint on device |
|---|------|-------|----------------------|
| 1 | [x] | 0.1–0.3 | builds from the script; log file appears |
| 2 | [ ] | 1.1–1.8 | language dialog OK; install 2 apps in a row shows success both times; search page 3; a redirected download finishes |
| 3 | [ ] | 1.9 + `lupdate` | strings are right in EN + one translation |
| 4 | [ ] | 2.1, 2.2 | all 5 lists load; airplane mode shows an error, not a spinner |
| 5 | [ ] | 3.1–3.5 | install state/update badges are the same as before for self-signed, unsigned and Store apps |
| 6 | [ ] | 4.1–4.4, 5.1–5.2 | Store flow, launch, cancel download, widget count |
| 7 | [ ] | 2.3, 6.x | settings survive an upgrade from 2.00(5) |
| 8 | [ ] | 7.x, 8.x | general UI pass |

Risky areas, which need extra device testing: the variant/upgrade logic (3.3; test the case
where an app has `uid == uidunsigned`), the Store-redirect event-filter flow (4.2), and
anything that touches the persisted `apps/*` keys (6.2).
