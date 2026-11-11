#ifndef VERSIONCOMPARE_H
#define VERSIONCOMPARE_H

#include <QString>

// Version strings as the SIS registry and the server write them ("major.minor.build").
// Pure Qt, so it builds and can be checked on desktop.

// Compares a and b part by part (split on '.'); returns -1 if a < b, 0 if equal, 1 if a > b.
// A missing part counts as 0, and each part is read up to its first non-digit
// (so "1.2" == "1.2.0", "1.10" > "1.9", "2.0.3b" == "2.0.3"):
//   compareVersions("1.2.3", "1.2.3")  ==  0
//   compareVersions("1.2.3", "1.2.4")  == -1
//   compareVersions("1.10.0", "1.9.9") ==  1
//   compareVersions("2", "1.99.99")    ==  1
//   compareVersions("1.2", "1.2.0")    ==  0
//   compareVersions("", "0.0.0")       ==  0
int compareVersions(const QString &a, const QString &b);

// True if available is newer than installed. False if either one is empty
// (empty installed = not installed, empty available = the server sent none):
//   isNewer("1.0.0", "1.0.1") == true
//   isNewer("1.0.1", "1.0.0") == false
//   isNewer("1.0.0", "1.0.0") == false
//   isNewer("", "1.0.0")      == false
//   isNewer("1.0.0", "")      == false
bool isNewer(const QString &installed, const QString &available);

#endif // VERSIONCOMPARE_H
