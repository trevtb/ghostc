#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QObject>
#include <QRegExp>
#include <QStringList>

class Validator : public QObject {
    Q_OBJECT

public:
    explicit Validator(QObject *parent = 0);
    
signals:
    
public slots:
    bool isValidMailAddress(QString);
    bool isInt(QString);
    bool isDouble(QString);
    bool isValidPort(QString);
    bool isValidAuthValue(QString);
    bool isValidEncryptionValue(QString);
    bool isStdDbString(QString);
    bool isStdDbInt(QString);
    bool isValidUserAttribute(QString, QString);
    bool isValidAccountAttribute(QString, QString);
    bool isEncryptedValue(QString);
    bool isValidUserString(QString);
    bool isValidMD5Hash(QString);
};

#endif // VALIDATOR_H
