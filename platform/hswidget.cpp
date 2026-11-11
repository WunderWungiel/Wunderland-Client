#include "hswidget.h"
#include <QDebug>
#include <QSettings>
#include <e32std.h> // RProcess

static const char *sw_type = "tworows";
static const char *sw_name = "Wunderland";
static const char *sw_id = "HSWidgetxWunderland";
static const QString sw_icon = QString("uid(%1)").arg(APP_UID); //App UID to set the widget icon

static const QString sw_image("image1");
static const QString row1("text1");
static const QString row2("text2");

HSWidget::HSWidget(UpdateTracker *_updateTracker, QObject *parent) :
    QObject(parent), widget(0), updateTracker(_updateTracker), waitingForCheck(false)
{
    connect(updateTracker, SIGNAL(checkFinished(int,int)),
            this, SLOT(checkFinished(int,int)));

    // Built after Language installs the translator, so the description is translated
    const QString desc = tr("With the Wunderland homescreen widget you will not miss any updates for your app. Visit the Wunderland help menu to see how it works.");
    widget = QHSWidget::create(sw_type, sw_name, sw_id, desc, sw_icon, this);
    if (!widget) {
        qWarning() << "HSWidget: cannot load the widget publisher DLL";
        return;
    }
    connect(widget, SIGNAL(handleEvent(QHSWidget*, QHSEvent)), this, SLOT(handleEvent(QHSWidget*, QHSEvent) ));
    connect(widget, SIGNAL(handleItemEvent(QHSWidget*, QString, QHSItemEvent)), this, SLOT(handleItemEvent(QHSWidget*, QString, QHSItemEvent)));
}

void HSWidget::registerWidget() {
    if (!widget) return;
    qDebug() << "HSWidget: register";
    widget->RegisterWidget();
}

void HSWidget::publishWidget() {
    if (!widget) return;
    widget->PublishWidget();
}

void HSWidget::removeWidget() {
    if (!widget) return;
    QSettings settings;
    settings.remove("widget");
    widget->RemoveWidget();
}

void HSWidget::handleEvent( QHSWidget* /*aSender*/, QHSEvent aEvent ) {
    switch(aEvent) {
        case EActivate:
            qDebug() << "HSWidget: activate";
            widget->SetItem(sw_image, "c:\system\Wunderland.png");
            refresh();
            break;
        case EResume:
            qDebug() << "HSWidget: resume";
            refresh();
            break;
        case EDeactivate: {
                qDebug() << "HSWidget: deactivate";
                QSettings settings;
                settings.remove("widget/lastUpdateTime");
            }
            break;
        default:
            break;
    }
}

void HSWidget::handleItemEvent( QHSWidget* /*aSender*/, QString aTemplateItemName, QHSItemEvent /*aEvent*/) {
    if (aTemplateItemName == sw_image)
        bringToFront();
}

void HSWidget::bringToFront() {
    // Not User::LeaveIfError: a leave in this slot would be an uncaught exception
    RProcess proc;
    TInt err = proc.Create(_L("Wunderland.exe"),KNullDesC);
    if (err != KErrNone) {
        qWarning() << "HSWidget: cannot start Wunderland.exe - error" << err;
        return;
    }
    proc.Resume();
    proc.Close();
}

void HSWidget::updateWidget( QString icon, QString nRow1, QString nRow2 ) {
    if (!widget) return;
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
    if (!widget) return;
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
