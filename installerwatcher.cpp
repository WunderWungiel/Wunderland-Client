#include "installerwatcher.h"
#include <QDebug>
#include <e32base.h>
#include <e32std.h>

namespace {
const int PollInterval = 1000;          // ms
// Without a running installserver seen yet, wait this long for it to show up
const int StartGrace = 10 * 1000;       // ms
// Give up on a poll that never sees installserver go away
const int PollTimeout = 30 * 60 * 1000; // ms
}

// Completes when the installer thread exits. RunL only queues a call to the
// watcher, so no Qt code runs inside the active scheduler.
class InstallerWatcher::Observer : public CActive
{
public:
    explicit Observer(InstallerWatcher *owner) :
        CActive(EPriorityStandard),
        m_owner(owner)
    {
        CActiveScheduler::Add(this);
    }

    ~Observer()
    {
        Cancel();
        m_thread.Close();
    }

    TInt Start(TUint64 threadId)
    {
        if (IsActive()) return KErrInUse;
        m_thread.Close();
        TInt err = m_thread.Open(TThreadId(threadId));
        if (err != KErrNone) return err;
        // Completes right away if the thread has already exited
        m_thread.Logon(iStatus);
        SetActive();
        return KErrNone;
    }

protected:
    void RunL()
    {
        qDebug() << "InstallerWatcher: installer thread exited, reason" << m_thread.ExitReason();
        m_thread.Close();
        QMetaObject::invokeMethod(m_owner, "onInstallerExited", Qt::QueuedConnection);
    }

    void DoCancel()
    {
        m_thread.LogonCancel(iStatus);
    }

private:
    InstallerWatcher *m_owner;
    RThread m_thread;
};

InstallerWatcher::InstallerWatcher(QObject *parent) :
    QObject(parent),
    m_observer(new Observer(this)),
    m_installServerSeen(false)
{
    m_pollTimer.setInterval(PollInterval);
    connect(&m_pollTimer, SIGNAL(timeout()), this, SLOT(poll()));
}

InstallerWatcher::~InstallerWatcher()
{
    delete m_observer;
}

void InstallerWatcher::watch(quint64 installerThreadId)
{
    m_observer->Cancel();
    m_pollTimer.stop();

    if (installerThreadId != 0) {
        TInt err = m_observer->Start(installerThreadId);
        if (err == KErrNone) {
            qDebug() << "InstallerWatcher: waiting for installer thread" << installerThreadId;
            return;
        }
        qWarning() << "InstallerWatcher: cannot log on to installer thread" << installerThreadId
                   << "- error" << err << "- polling instead";
    }
    startPolling(false);
}

void InstallerWatcher::onInstallerExited()
{
    // The installer may have handed the installation to installserver
    if (installServerRunning()) {
        qDebug() << "InstallerWatcher: installserver still running, polling";
        startPolling(true);
        return;
    }
    finish();
}

void InstallerWatcher::startPolling(bool installServerSeen)
{
    m_installServerSeen = installServerSeen;
    m_pollStarted.start();
    m_pollTimer.start();
}

void InstallerWatcher::poll()
{
    if (installServerRunning()) {
        m_installServerSeen = true;
        if (m_pollStarted.elapsed() < PollTimeout) return;
        qWarning() << "InstallerWatcher: installserver still running after"
                   << PollTimeout / 1000 << "s, giving up";
    } else if (!m_installServerSeen && m_pollStarted.elapsed() < StartGrace) {
        return; // not started yet
    }
    finish();
}

void InstallerWatcher::finish()
{
    m_pollTimer.stop();
    qDebug() << "InstallerWatcher: installer closed";
    emit finished();
}

bool InstallerWatcher::installServerRunning()
{
    // Process full names look like "installserver[101f875a]0001"
    TFindProcess find(_L("installserver*"));
    TFullName name;
    return find.Next(name) == KErrNone;
}
