#ifndef CRYPTOHELPER_H
#define CRYPTOHELPER_H

#include <QObject>
#include <QCryptographicHash>
#include <QDateTime>
#include <QMap>
#include <QtCrypto>
#include <QDebug>
#include <QTextCodec>

class CryptoHelper : public QObject {
    Q_OBJECT

private:
    QCA::SecureArray key;
    QString algorithm;
    int bitsize;

public:
    explicit CryptoHelper(QObject *parent = 0);
    CryptoHelper(QString algorithm, int bitsize, QObject *parent = 0);
    
signals:

private:
    QCA::SecureArray generateIV();
    QCA::SecureArray keyStringToSecureArray(QString);
    QStringList separateEncPackage(QString);

public slots:
    QString getNewAPIKey();
    QString encrypt(QString);
    QString decrypt(QString);
    QString generateKey();
    void setKey(QCA::SecureArray);
    QString getPasswordHash(QString);

};

#endif // CRYPTOHELPER_H
