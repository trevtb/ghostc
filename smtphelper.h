#ifndef SMTPHELPER_H
#define SMTPHELPER_H

#include <QObject>
#include <smtpclient/SmtpMime>
#include <dbhelper.h>
#include <QStringList>
#include <QThread>
#include <QtGlobal>
#include <QWaitCondition>
#include <QMutex>
#include <limits.h>

class SmtpHelper : public QObject {
    Q_OBJECT

private:
    QString host;
    int port;
    int auth;
    int ssl;
    QString user;
    QString pass;
    QString from;
    int recipients_per_mail;
    int send_delay;
    QString subject;
    QString message;
    QMap<QString, QString> *settings;
    void initialize();

public:
    explicit SmtpHelper(QObject *parent = 0);
    SmtpHelper(QMap<QString, QString> *settings, QObject *parent = 0);
    
signals:
    
private slots:
    QStringList getRecipients();
    QList<QString> getMailAccount();

public slots:
    bool sendMails(QString);
    bool sendNotify(QStringList, QString, QString, bool);
    QList<QStringList> getRecipientArray(QStringList, int);

};

#endif // SMTPHELPER_H
