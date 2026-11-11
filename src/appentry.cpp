// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appentry.h"

QHash<int, QByteArray> appRoleNames()
{
    static QHash<int, QByteArray> names;
    if (names.isEmpty()) {
        names[IdRole] = "id";
        names[NameRole] = "name";
        names[UidRole] = "uid";
        names[IconRole] = "icon";
        names[VersionRole] = "version";
        names[VersionDateRole] = "versiondate";
        names[InstalledRole] = "installed";
        names[CategoryRole] = "category";
        names[LanguageRole] = "language";
        names[OsRole] = "os";
        names[DeveloperRole] = "developer";
        names[MailRole] = "mail";
        names[WebsiteRole] = "website";
        names[TwitterRole] = "twitter";
        names[FacebookRole] = "facebook";
        names[DonationRole] = "donation";
        names[AasRole] = "aas";
        names[PriceRole] = "price";
        names[DescriptionRole] = "description";
        names[Image1Role] = "image1";
        names[Image2Role] = "image2";
        names[Image3Role] = "image3";
        names[TagsRole] = "tags";
        names[ChangelogRole] = "changelog";
        names[DownloadRole] = "download";
        names[UpdateAvailableRole] = "updateAvailable";
        names[Image4Role] = "image4";
        names[Image5Role] = "image5";
        names[DownloadSizeRole] = "downloadsize";
    }
    return names;
}

AppEntry::AppEntry()
    : id(0), category(0), price(0.0), installed(false), updateAvailable(false)
{
}

AppEntry AppEntry::fromFields(const QHash<QString, QString> &fields)
{
    AppEntry e;
    e.id = fields.value("id").toInt();
    e.name = fields.value("name");
    e.uid = fields.value("uid");
    e.icon = fields.value("icon");
    e.version = fields.value("version");
    e.versionDate = QDateTime::fromString(fields.value("versiondate"), "yyyy-MM-dd hh:mm:ss");
    e.category = fields.value("category").toInt();
    e.language = fields.value("language");
    e.os = fields.value("os");
    e.developer = fields.value("developer");
    e.mail = fields.value("mail");
    e.website = fields.value("website");
    e.twitter = fields.value("twitter");
    e.facebook = fields.value("facebook");
    e.donation = fields.value("donation");
    e.aas = fields.value("aas");
    e.price = fields.value("price").toDouble();
    e.description = fields.value("description");
    e.image1 = fields.value("image1");
    e.image2 = fields.value("image2");
    e.image3 = fields.value("image3");
    e.image4 = fields.value("image4");
    e.image5 = fields.value("image5");
    e.tags = fields.value("tags");
    e.changelog = fields.value("changelog");
    e.download = fields.value("download");
    e.downloadSize = fields.value("downloadsize");
    return e;
}

QVariant AppEntry::data(int role) const
{
    switch (role) {
    case IdRole: return id;
    case NameRole: return name;
    case UidRole: return uid;
    case IconRole: return icon;
    case VersionRole: return version;
    case VersionDateRole: return versionDate;
    case InstalledRole: return installed;
    case CategoryRole: return category;
    case LanguageRole: return language;
    case OsRole: return os;
    case DeveloperRole: return developer;
    case MailRole: return mail;
    case WebsiteRole: return website;
    case TwitterRole: return twitter;
    case FacebookRole: return facebook;
    case DonationRole: return donation;
    case AasRole: return aas;
    case PriceRole: return price;
    case DescriptionRole: return description;
    case Image1Role: return image1;
    case Image2Role: return image2;
    case Image3Role: return image3;
    case TagsRole: return tags;
    case ChangelogRole: return changelog;
    case DownloadRole: return download;
    case UpdateAvailableRole: return updateAvailable;
    case Image4Role: return image4;
    case Image5Role: return image5;
    case DownloadSizeRole: return downloadSize;
    }
    return QVariant();
}
