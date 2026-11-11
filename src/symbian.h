#ifndef SYMBIAN_H
#define SYMBIAN_H

#include <QObject>
#include <QString>

// QML helpers for native functions (the symbianFunctions context property).
// The native calls are in platform/.
class Symbian : public QObject
{
    Q_OBJECT
public:
    Symbian(QObject *parent = 0);

    // Opens url in the browser selected in the phone settings, not always the
    // built-in one as Qt.openUrlExternally() does. False (and a log line) if
    // the browser could not be started.
    Q_INVOKABLE bool openUrlInDefaultBrowser(QString url);
};

#endif // SYMBIAN_H
