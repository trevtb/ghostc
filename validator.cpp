#include "validator.h"

Validator::Validator(QObject *parent) : QObject(parent) {
} //endconstructor

bool Validator::isValidMailAddress(QString address) {
    QRegExp mailREX("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
    mailREX.setCaseSensitivity(Qt::CaseInsensitive);
    mailREX.setPatternSyntax(QRegExp::RegExp);

    if (mailREX.exactMatch(address) && isStdDbString(address)) {
        return true;
    } else {
        return false;
    } //endif
} //endfunction isValidMailAddress

bool Validator::isInt(QString num) {
    bool check = false;
    num.toInt(&check);

    return check;
} //endfunction isInt

bool Validator::isDouble(QString num) {
    bool check = false;
    num.toDouble(&check);

    return check;
} //endfunction isDouble

bool Validator::isValidPort(QString port) {
    if (isInt(port) && port.toInt() > 0 && port.toInt() < 65534) {
        return true;
    } else {
        return false;
    } //endif
} //endfunction isValidPort

bool Validator::isValidAuthValue(QString aval) {
    if (isInt(aval) && (aval.toInt() == 0 || aval.toInt() == 1)) {
        return true;
    } else {
        return false;
    } //endif
} //endfunction isValidAuthValue

bool Validator::isValidEncryptionValue(QString encval) {
    if (isInt(encval) && encval.toInt() >= 0 && encval.toInt() < 3) {
        return true;
    } else {
        return false;
    } //endif
} //endfunction isValidEncryptionValue

bool Validator::isStdDbString(QString str) {
    if (str.length() > 0 && str.length() <= 128) {
       return true;
    } else {
        return false;
    } //endif
} //endfunction isValidStdDbString

bool Validator::isStdDbInt(QString mr) {
    if (isInt(mr) && mr.toInt() >= 0 && mr.toInt() <= 999999999) {
        return true;
    } else {
        return false;
    } //endif
} //endfunction isValidMaxRecipients

bool Validator::isEncryptedValue(QString val) {
    if (isDouble(val)) {
        return false;
    } else {
        return true;
    } //endif
} //endfunction isEncryptedValue

bool Validator::isValidUserAttribute(QString attr, QString value) {
    if ((QString::compare(attr, "email") == 0 && isValidMailAddress(value)) ||
     (QString::compare(attr, "username") == 0 && isStdDbString(value)) ||
     (QString::compare(attr, "password") == 0 && isStdDbString(value))) {
            return true;
    } else {
        return false;
    } //endif
} //endfunction isValidUserAttribute

bool Validator::isValidAccountAttribute(QString attr, QString value) {
    if ((QString::compare(attr, "host") == 0 && isStdDbString(value)) ||
        (QString::compare(attr, "port") == 0 && isValidPort(value)) ||
        (QString::compare(attr, "auth") == 0 && isValidAuthValue(value)) ||
        (QString::compare(attr, "ssl_tls") == 0 && isValidEncryptionValue(value)) ||
        (QString::compare(attr, "username") == 0 && isStdDbString(value)) ||
        (QString::compare(attr, "password") == 0 && isStdDbString(value)) ||
        (QString::compare(attr, "email") == 0 && isValidMailAddress(value)) ||
        (QString::compare(attr, "max_recipients") == 0 && isStdDbInt(value)) ||
        (QString::compare(attr, "send_delay") == 0 && isStdDbInt(value))) {
        return true;
    } else {
        return false;
    } //endif
} //endfunction isValidUserAttribute

bool Validator::isValidUserString(QString user) {
    bool retVal = true;

    if (user.length() > 32 || user.length() < 1) {
        retVal = false;
    } //endif

    QRegExp rx("^[\s]+$");
    rx.setPatternSyntax(QRegExp::RegExp);
    if (rx.exactMatch(user)) {
        retVal = false;
    } //endif

    QRegExp rx2("^[a-zA-Z0-9-_]+$");
    if (!rx2.exactMatch(user)) {
        retVal = false;
    } //endif

    return retVal;
} //endfunction isValidUserString

bool Validator::isValidMD5Hash(QString hash) {
    bool retVal = true;

    if (hash.length() != 32) {
        retVal = false;
    } //endif

    QRegExp rx("^[a-zA-Z0-9]+$");
    if (!rx.exactMatch(hash)) {
        retVal = false;
    } //endif

    return retVal;
} //endfunction isValidMD5Hash
