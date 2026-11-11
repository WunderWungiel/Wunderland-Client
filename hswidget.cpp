#include "hswidget.h"
#include <QDebug>
#include <QSettings>
#include <apgcli.h> // for RApaLsSession
#include <apacmdln.h> // for CApaCommandLine
#include <apgtask.h> // bring to foreground
#include <eikenv.h> // bring to foreground

const QString sw_type = "tworows";
const QString sw_name = "Wunderland";
const QString sw_desc = "With the Wunderland homescreen widget you will not miss any updates for your app. Visit the Wunderland help menu to see how it works.";
const QString sw_id = "HSWidgetxWunderland";
const QString sw_icon = QString("uid(%1)").arg(APP_UID); //App UID to set the widget icon

const QString sw_image ("image1");
const QString row1 ("text1");
const QString row2 ("text2");

HSWidget::HSWidget(UpdateTracker *_updateTracker, QObject *parent) :
    QObject(parent), updateTracker(_updateTracker), waitingForCheck(false)
{
    connect(updateTracker, SIGNAL(checkFinished(int,int)),
            this, SLOT(checkFinished(int,int)));

    qDebug() << "init";

    //wunderlandVersion = QApplication::instance()->applicationVersion();

    widget = QHSWidget::create(sw_type, sw_name, sw_id, sw_desc, sw_icon, this);
    connect(widget, SIGNAL(handleEvent(QHSWidget*, QHSEvent)), this, SLOT(handleEvent(QHSWidget*, QHSEvent) ));
    connect(widget, SIGNAL(handleItemEvent(QHSWidget*, QString, QHSItemEvent)), this, SLOT(handleItemEvent(QHSWidget*, QString, QHSItemEvent)));
}

void HSWidget::registerWidget() {
    qDebug() << "register";
    widget->RegisterWidget();
}

void HSWidget::publishWidget() {
    qDebug() << "loadList";
    //loadList();
    qDebug() << "publish";
    widget->PublishWidget();
}

void HSWidget::removeWidget() {
    qDebug() << "remove";
    QSettings settings("WunderWungiel", "Wunderland");
    settings.remove("widget");
    //settings.sync();
    widget->RemoveWidget();
}

void HSWidget::handleEvent( QHSWidget* /*aSender*/, QHSEvent aEvent ) {
    qDebug() << "handleEvent";
    switch(aEvent) {
        case EActivate: {
                qDebug() << "handleActivate";
                widget->SetItem(sw_image, "c:\\system\\Wunderland.png");
                //updateWidget("c:\\data\\Images\\Pictures\\QtSunny2.JPG", "a", "b");
                //publishWidget();
                refresh();
            }
            break;
        case EResume: {
                qDebug() << "handlePublish";
                //publishWidget();
                refresh();
                qDebug() << "handlePublishAfter";
            }
            break;
        case EDeactivate: {
                qDebug() << "handleDeactivate";
                QSettings settings("WunderWungiel", "Wunderland");
                settings.remove("widget/lastUpdateTime");
            }
            break;
        default:
            break;
    }
}

void HSWidget::handleItemEvent( QHSWidget* /*aSender*/, QString aTemplateItemName, QHSItemEvent aEvent) {
    qDebug() << "handleItemEvent";
    if(aTemplateItemName.compare(sw_image)==0) {
        this->bringToFront();
        //updateWidget("c:\\data\\Images\\Pictures\\QtSunny2.JPG", "a", "b");
        //publishWidget();
        //refresh();
    }
}

void HSWidget::bringToFront() {
    /*TApaTask task( CEikonEnv::Static()->WsSession() );
    task.SetWgId(CEikonEnv::Static()->RootWin().Identifier());
    task.BringToForeground();*/

    RProcess proc;
    User::LeaveIfError(proc.Create(_L("Wunderland.exe"),KNullDesC));
    proc.Resume();
    proc.Close();

    /*TApaTaskList taskList(CCoeEnv::Static()->WsSession());
    const TUid aAppUid = TUid::Uid(0xA1331002);
    TApaTask task = taskList.FindApp(aAppUid);
    if (task.Exists()) {
        task.BringToForeground();
    }

    TApaTaskList taskList(CCoeEnv::Static()->WsSession());
    TApaTask task = taskList.FindApp(aAppUid);
    if (task.Exists()) {
        task.BringToForeground();
    } else {
        RApaLsSession apaLsSession;
        User::LeaveIfError(apaLsSession.Connect());
        CleanupClosePushL(apaLsSession);

        TApaAppInfo appInfo;
        TInt retVal = apaLsSession.GetAppInfo(appInfo, aAppUid);

        if(retVal == KErrNone) {
            CApaCommandLine* cmdLine = CApaCommandLine::NewLC();
            cmdLine->SetExecutableNameL(appInfo.iFullName);
            cmdLine->SetCommandL(EApaCommandRun);
            User::LeaveIfError( apaLsSession.StartApp(*cmdLine) );

            TApaTask task = taskList.FindApp(aAppUid);
            if (task.Exists()) {
                task.BringToForeground();
            }

            CleanupStack::PopAndDestroy(cmdLine);
        }

        CleanupStack::PopAndDestroy(&apaLsSession);
    }*/
}

void HSWidget::updateWidget( QString icon, QString nRow1, QString nRow2 ) {
    widget->SetItem(sw_image, icon);
    widget->SetItem(row1, nRow1);
    widget->SetItem(row2, nRow2);
    publishWidget();
}

QString HSWidget::updatesText(int updates) const {
    if (updates == 1) return tr("1 update available");
    return tr("%1 updates available").arg(updates);
}

QString HSWidget::newAppsText(int newApps) const {
    if (newApps == 1) return tr("1 new app");
    return tr("%1 new apps").arg(qMax(newApps, 0));
}

void HSWidget::dataUpdated(int updates, int newApps) {
    widget->SetItem(row1, updatesText(updates));
    widget->SetItem(row2, newAppsText(newApps));

    publishWidget();
}

void HSWidget::refresh() {
    if (updateTracker->lastCheckOlderThanADay()) {
        waitingForCheck = true;
        updateTracker->check();
    } else {
        dataUpdated(updateTracker->count(), 0);
    }
}

void HSWidget::checkFinished(int updates, int newApps) {
    // The app's own checks don't publish the widget
    if (!waitingForCheck) return;
    waitingForCheck = false;
    dataUpdated(updates, newApps);
}
