#ifndef HSWIDGET_H
#define HSWIDGET_H

#include <QObject>
#include "qhswidget.h"
#include "updatetracker.h"

class HSWidget : public QObject
{
    Q_OBJECT
public:
    explicit HSWidget(UpdateTracker *updateTracker, QObject *parent = 0);
    Q_INVOKABLE void registerWidget();
    Q_INVOKABLE void publishWidget();
    Q_INVOKABLE void removeWidget();
    Q_INVOKABLE void updateWidget( QString icon, QString nRow1, QString nRow2 );
    void bringToFront();

public slots:
    void handleEvent(QHSWidget*, QHSEvent aEvent );
    void handleItemEvent(QHSWidget*, QString aTemplateItemName,
                         QHSItemEvent aEvent);

    void checkFinished(int updates, int newApps);
private:
    // Shows the update count; checks the server first if the last check is a day old
    void refresh();
    void dataUpdated(int updates, int newApps);
    QString updatesText(int updates) const;
    QString newAppsText(int newApps) const;

    QHSWidget* widget;

    UpdateTracker *updateTracker;
    // The widget asked for the running check, so it shows the result
    bool waitingForCheck;

};

#endif // HSWIDGET_H
