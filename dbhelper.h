#ifndef DBHELPER_H
#define DBHELPER_H

#include <QObject>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStringList>
#include <cryptohelper.h>
#include <validator.h>
#include <iohelper.h>
#include <ctime>

class DbHelper : public QObject {
    Q_OBJECT
public:
    explicit DbHelper(QObject *parent = 0);
    DbHelper(QMap<QString, QString> *settings, QObject *parent = 0);

private:
    QString dbhost;
    QString dbport;
    QString dbname;
    QString dbuser;
    QString dbpass;
    QSqlDatabase db;
    CryptoHelper *crypt;
    QStringList tablenames;
    QMap<QString, QStringList> dbstruct;
    
signals:

private slots:
    bool userExists(QString, QString);
    bool mailAccountExists(QString);
    bool isValidTable(QString);
    bool isValidAttribute(QString, QString);
    bool supportsMySQL();

public slots:
    bool connectToDb();
    void closeDbConnection();
    QList<QList<QString> > getTable(QString);
    QList<QList<QString> > getDecryptedTable(QString);
    void updateKey(QString);
    void incrementAccountCounter(int);
    void addUser(QString, QString, QString);
    void addMailAccount(QString, int, int, int, QString, QString, QString, int, int);
    void delUser(int);
    void delMailAccount(int);
    void update(QString, int, QString, QString);
    void doInstall();
    QString reencrypt();
    QList<QString> getDecryptedUser(QString);
    QList<QString> getDecryptedUser(int);
    QList<QList<QString> > getUserRequests(QString);
    QList<QString> getUserRequest(int);
    QString writeNewRequest(QString, QString, QString);
    bool userExists(QString);
    void deleteRequest(int);
    void enterChat(QString, int, QString);
    QList<QString> getRequestProtectParams(QString, int);
    void addChatMessage(int, int, int, int, QString, int);
    QList<QList<QString> > getDecryptedChatMessages(int);
    void cleanChatHistory(int);
    void deleteChat(int);
    void nullAccountCounters();
    QString getRequestPassword(int);
};

#endif // DBHELPER_H
