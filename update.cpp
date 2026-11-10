#include "update.h"

Update::Update(StoreApi *_api, QObject *parent) :
    QObject(parent)
{
    api = _api;
    fileUrl = "";

    connect(&downloader, SIGNAL(progressChanged(int)),
            this, SLOT(progressChangedReceived(int)));
}

void Update::loadUpdateData(bool all) {
    ApiReply *reply = api->get(api->url(all ? "/changelog.xml" : "/version.xml"));
    reply->setProperty("all", all);
    connect(reply, SIGNAL(finished()),
            this, SLOT(parseData()));
}

void Update::startDownload() {
    downloader.startDownloadWithoutStats(fileUrl);
}

void Update::parseData() {
    ApiReply *reply = qobject_cast<ApiReply *>(sender());
    if (!reply) return; // deletes itself
    const bool all = reply->property("all").toBool();

    // An empty body takes the "could not be loaded" path below
    QString html;
    if (reply->error() == StoreApi::NoError) html = reply->text();

    if (html.length() > 0) {
        int lastPos = html.indexOf("<url>") + 5;
        fileUrl = html.mid(lastPos, html.indexOf("</url>", lastPos)-lastPos);

        lastPos = html.indexOf("<message>") + 9;
        if (all) {
            _changelog = html.mid(lastPos, html.indexOf("</message>", lastPos)-lastPos);
            emit changelogChanged();
        } else {
            emit loadingDone(true, html.mid(lastPos, html.indexOf("</message>", lastPos)-lastPos));
        }
    } else {
        if (all) {
            emit changelogChanged();
        } else {
            emit loadingDone(false, "");
        }
    }
}
