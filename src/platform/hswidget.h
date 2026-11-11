// SPDX-FileCopyrightText: Sebastian (schumi1331)
// SPDX-FileCopyrightText: 2026 Wunder Wungiel
// SPDX-License-Identifier: GPL-3.0-or-later

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
    Q_INVOKABLE void publishWidget();
    Q_INVOKABLE void removeWidget();
    Q_INVOKABLE void updateWidget( QString icon, QString nRow1, QString nRow2 );

public slots:
    // A slot so main.cpp can run it after the UI is up (it is slow, and showed as splash delay)
    void registerWidget();
    void handleEvent(QHSWidget*, QHSEvent aEvent );
    void handleItemEvent(QHSWidget*, QString aTemplateItemName,
                         QHSItemEvent aEvent);

    void checkFinished(int updates, int newApps);
private:
    void bringToFront();
    // Shows the update count; checks the server first if the last check is a day old
    void refresh();
    void dataUpdated(int updates, int newApps);
    QString updatesText(int updates) const;
    QString newAppsText(int newApps) const;

    // 0 when the publisher DLL could not be loaded
    QHSWidget* widget;

    UpdateTracker *updateTracker;
    // The widget asked for the running check, so it shows the result
    bool waitingForCheck;

};

#endif // HSWIDGET_H
