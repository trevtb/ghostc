#ifndef GMAILER_H
#define GMAILER_H

#include "dbhelper.h"
#include "smtphelper.h"
#include <QObject>
#include <QDebug>

class GMailer : public QObject {
    Q_OBJECT
public:
    explicit GMailer(QObject *parent = 0);
    GMailer(QMap<QString, QString> *settings, QObject *parent = 0);

private:
    QMap<QString, QString> *settings;

signals:
    
public slots:
    bool sendEmails();
    bool notify(QStringList, QString, QString, bool);
};

#endif // GMAILER_H
