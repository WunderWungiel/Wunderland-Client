#ifndef SYMBIAN_H
#define SYMBIAN_H

#include <QObject>
#include <QSystemInfo>

/*
Includes of Symbian libraries. If you are building for multiple OSes, use
#ifdef Q_OS_SYMBIAN
...
#endif

DON'T FORGET TO ADD LIBS TO THE PRO FILE OF YOUR PROJECT
*/
//General
#include <eikenv.h>
//openUrlInDefaultBrowser
#include <apgcli.h>
#include <apgtask.h>

class Symbian : public QObject
{
    Q_OBJECT
public:
    Symbian(QObject *parent = 0);

    // False (and a log line) if the browser could not be started
    Q_INVOKABLE bool openUrlInDefaultBrowser(QString url);

private:
    // These leave on errors; only openUrlInDefaultBrowser() calls them, inside a TRAP
    void openUrlL(const TDesC& aUrl);
    void LaunchBrowserL(const TDesC& aUrl, TUid& _id);
};

#endif // SYMBIAN_H
