#ifndef INSTALLERWATCHER_H
#define INSTALLERWATCHER_H

#include <QObject>
#include <QTime>
#include <QTimer>

// Tells when the installer opened by AppManagement::openInstaller() has closed.
// Runs on the UI thread: it logs on to the installer thread that StartDocument
// returned (RThread::Logon on an active object). If there is no thread to log on
// to, or the installer handed the work to a separate installserver process that
// is still running, it falls back to checking once per second whether an
// installserver process is running.
class InstallerWatcher : public QObject
{
    Q_OBJECT
public:
    explicit InstallerWatcher(QObject *parent = 0);
    ~InstallerWatcher();

    // installerThreadId: the thread id from openInstaller(), 0 if unknown.
    // Emits finished() once when the installer has closed.
    void watch(quint64 installerThreadId);

signals:
    void finished();

private slots:
    void onInstallerExited();
    void poll();

private:
    class Observer;

    void startPolling(bool installServerSeen);
    void finish();
    static bool installServerRunning();

    Observer *m_observer;
    QTimer m_pollTimer;
    QTime m_pollStarted;
    bool m_installServerSeen;
};

#endif // INSTALLERWATCHER_H
