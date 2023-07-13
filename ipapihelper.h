#ifndef IPAPIHELPER_H
#define IPAPIHELPER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QScriptEngine>
#include <QScriptValue>

class IpApiHelper : public QObject {
    Q_OBJECT

private:
    QString urlstring;
    QNetworkAccessManager netManager;

public:
    explicit IpApiHelper(QObject *parent = 0);
    
signals:
    
public slots:
    QString getExternalIP();
};

#endif // IPAPIHELPER_H
