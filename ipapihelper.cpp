#include "ipapihelper.h"

IpApiHelper::IpApiHelper(QObject *parent) : QObject(parent) {
    urlstring = "http://ip-api.com/json";
} //endconstructor

QString IpApiHelper::getExternalIP() {
    QNetworkRequest request;
    request.setUrl(QUrl(urlstring));
    request.setRawHeader("User-Agent", "GHOST v0.1");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QNetworkReply *reply = netManager.get(request);
    QEventLoop eLoop;

    connect(reply, SIGNAL(finished()), &eLoop, SLOT(quit()));
    eLoop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug("ERROR: %s", reply->errorString().toStdString().c_str());
        return QString("error");
    } //endif

    QString data = (QString) reply->readAll();

    QScriptEngine engine;
    QScriptValue result = engine.evaluate("(" + data + ")");

    QString ip = result.property("query").toString();

    return ip;
} //endfunction getExternalIP
