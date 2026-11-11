#ifndef APPENTRY_H
#define APPENTRY_H

#include <Qt>
#include <QString>
#include <QHash>
#include <QByteArray>
#include <QDateTime>
#include <QVariant>

// Model roles of an app list (AppCatalog's AppListModels). QML reads them by the names in
// appRoleNames(). The gaps (4, 5, 8, 9, 11, 12, 31, 33, 34, 39) were the removed
// unsigned/Store variant roles; keep the numbers stable.
enum AppRole {
    IdRole = Qt::UserRole + 1,
    NameRole = Qt::UserRole + 2,
    UidRole = Qt::UserRole + 3,
    IconRole = Qt::UserRole + 6,
    VersionRole = Qt::UserRole + 7,
    VersionDateRole = Qt::UserRole + 10,
    InstalledRole = Qt::UserRole + 13,
    CategoryRole = Qt::UserRole + 14,
    LanguageRole = Qt::UserRole + 15,
    OsRole = Qt::UserRole + 16,
    DeveloperRole = Qt::UserRole + 17,
    MailRole = Qt::UserRole + 18,
    WebsiteRole = Qt::UserRole + 19,
    TwitterRole = Qt::UserRole + 20,
    FacebookRole = Qt::UserRole + 21,
    DonationRole = Qt::UserRole + 22,
    AasRole = Qt::UserRole + 23,
    PriceRole = Qt::UserRole + 24,
    DescriptionRole = Qt::UserRole + 25,
    Image1Role = Qt::UserRole + 26,
    Image2Role = Qt::UserRole + 27,
    Image3Role = Qt::UserRole + 28,
    TagsRole = Qt::UserRole + 29,
    ChangelogRole = Qt::UserRole + 30,
    DownloadRole = Qt::UserRole + 32,
    UpdateAvailableRole = Qt::UserRole + 35,
    Image4Role = Qt::UserRole + 36,
    Image5Role = Qt::UserRole + 37,
    DownloadSizeRole = Qt::UserRole + 38
};

// Role number -> name used in QML delegates and AppListModel::get().
QHash<int, QByteArray> appRoleNames();

// One app as sent by applist.php, plus the client-side install state.
struct AppEntry
{
    AppEntry();

    // Builds an entry from the text of the child elements of one <app>, by tag name.
    // Missing fields are empty/0. installed and updateAvailable are false.
    static AppEntry fromFields(const QHash<QString, QString> &fields);

    // The value for a model role, typed as the QML pages expect it
    // (updateAvailable as a string, downloadSize as text).
    QVariant data(int role) const;

    // From the server
    int id;
    QString name;
    QString uid;
    QString icon;
    QString version;
    QDateTime versionDate;
    int category;
    QString language;
    QString os;
    QString developer;
    QString mail;
    QString website;
    QString twitter;
    QString facebook;
    QString donation;
    QString aas;
    double price;
    QString description;
    QString image1;
    QString image2;
    QString image3;
    QString image4;
    QString image5;
    QString tags;
    QString changelog;
    QString download;
    QString downloadSize;   // bytes, as sent

    // Client state
    bool installed;
    bool updateAvailable;
};

#endif // APPENTRY_H
