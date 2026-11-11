#ifndef PLATFORM_DEVICE_H
#define PLATFORM_DEVICE_H

#include <QString>

// Device information, from Qt Mobility (systeminfo) on Symbian. Desktop Qt has
// no Mobility, so other platforms use plain Qt.
namespace Platform {

// OS version as sent to the server in the "os" parameter
QString osVersion();

// Device language as a lower-case ISO 639-1 code ("en", "pl", ...)
QString deviceLanguage();

// On Symbian "<drive>/download/Wunderland" on F:, E: or C:/Data (in this
// order), the first one that exists, can be created and has more than minFree
// bytes free; elsewhere ~/.wunderland/download. Empty if none.
QString downloadFolder(qint64 minFree = 0);

} // namespace Platform

#endif // PLATFORM_DEVICE_H
