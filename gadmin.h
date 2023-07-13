#ifndef GADMIN_H
#define GADMIN_H

#include <QObject>
#include <QUrl>
#include <iostream>
#include <dbhelper.h>
#include <iohelper.h>

class GAdmin : public QObject {
    Q_OBJECT

private:
    QMap<QString, QString> *settings;

public:
    explicit GAdmin(QObject *parent = 0);
    GAdmin(QMap<QString,QString> *settings, QObject *parent = 0);

signals:
    
public slots:
    void printUserInfo();
    void printAccountInfo();
    void printOpenRequests(QString);
    void addUser(QString, QString, QString);
    void delUser(int);
    void addMailAccount(QString, QString, QString, QString, QString, QString, QString, QString, QString);
    void delMailAccount(int);
    void updateUser(int, QString, QString);
    void updateAccount(int, QString, QString);
    void doInstall();
    void reencrypt();
    void checkLogin(QString, QString);
    void printAPIKey();
    QString writeNewRequest(QString, QString, QString);
    bool userExists(QString);
    void changeSettings(QString, QString, QString);
    void enterChat(QString, QString, QString);
    void addChatMessage(int, QString, QString, int, bool);
    void printChatMessages(int);
    bool deleteChat(QString);
    void cleanTimeouts();
    QString getCurrentAPIURL();
    void printRequestPassword(QString);

};

#endif // GADMIN_H
