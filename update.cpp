#include "update.h"
#include "applistparser.h"

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

    // An empty or broken body takes the "could not be loaded" path below
    QHash<QString, QString> fields;
    if (reply->error() == StoreApi::NoError) {
        QString error;
        fields = AppListParser::parseFields(reply->text(), &error);
        if (!error.isEmpty()) {
            qWarning() << "Update: bad XML:" << error;
            fields.clear();
        }
    }

    if (fields.contains("message")) {
        if (all) {
            // changelog.xml has no <url>: keep the one from version.xml
            _changelog = fields.value("message");
            emit changelogChanged();
        } else {
            fileUrl = fields.value("url");
            emit loadingDone(true, fields.value("message"));
        }
    } else {
        if (all) {
            emit changelogChanged();
        } else {
            emit loadingDone(false, "");
        }
    }
}
