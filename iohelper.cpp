#include "iohelper.h"

IOHelper::IOHelper(QObject *parent) : QObject(parent) {
} //endconstructor

QString IOHelper::dirpath = "/etc/ghost/";
QString IOHelper::cfgname = "ghost.cfg";
QString IOHelper::keyname = "key.key";

QMap<QString, QString> IOHelper::readSettings() {
    QMap<QString, QString> settings;

    QStringList pparams, optparams;
    pparams << "db_host" << "db_name" << "db_user" << "db_pass" << "ghostphp_host" << "request_timeout";
    optparams << "db_port" << "tor_port";

    QFile file(IOHelper::dirpath + IOHelper::cfgname);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QStringList fcont;
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString str = in.readLine();
            if (str.length() > 0) {
                fcont << str.trimmed();
            } //endif
        } //endwhile

        foreach (QString line, fcont) {
            bool haseq = line.contains("=", Qt::CaseSensitive);
            bool iscomment = line.at(0).toLatin1() == '#';
            bool valargcount = (line.split("=").count() == 2) ? true : false;

            if (haseq && !iscomment && valargcount) {
                QStringList lineargs = line.split("=");
                foreach (QString param, pparams) {
                    if (QString::compare(param, lineargs[0].trimmed()) == 0) {
                        settings.insert(lineargs[0].trimmed(), lineargs[1].trimmed());
                    } //endif
                } //endforeach

                foreach (QString param, optparams) {
                    if (QString::compare(param, lineargs[0].trimmed()) == 0) {
                        settings.insert(lineargs[0].trimmed(), lineargs[1].trimmed());
                    } //endif
                } //endforeach
            } else if (!iscomment && (!haseq || !valargcount)) {
                qDebug("ERROR: Your config file contains invalid or is missing required values.");
                exit(0);
            } //endif
        } //endforeach
    } else {
        if (file.exists()) {
            qDebug("ERROR: No permission to read config file.");
        } else {
            qDebug("ERROR: Config file does not exist.");
        } //endif
    } //endif

    return settings;
} //endfunction readSettings

QMap<QString, QString> IOHelper::readEncryptedSettings() {
    CryptoHelper *crypt = new CryptoHelper("aes", 256);
    crypt->setKey(readKey());
    QMap<QString, QString> settings = readSettings();
    QMap<QString, QString> encrypted;
    QMap<QString, QString>::Iterator iter;
    iter = settings.begin();
    while (iter != settings.end()) {
        encrypted.insert(iter.key(), crypt->encrypt(iter.value()));
        iter++;
    } //endwhile

    return encrypted;
} //endfunction readEncryptedSettings

bool IOHelper::settingsReadable() {
    QFile setFile(IOHelper::dirpath + IOHelper::cfgname);
    QFileInfo finfo(IOHelper::dirpath + IOHelper::cfgname);

    if (setFile.exists() && finfo.isReadable()) {
        return true;
    } else {
        return false;
    } //endif
} //endfunction settingsReadable

bool IOHelper::keyFileValid() {
    QFile keyFile(IOHelper::dirpath + IOHelper::keyname);
    QFileInfo finfo(IOHelper::dirpath + IOHelper::keyname);

    if (keyFile.exists() && finfo.isReadable()) {
        keyFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QStringList fcont;
        QTextStream in(&keyFile);
        while (!in.atEnd()) {
            QString str = in.readLine();
            if (str.length() > 0) {
                fcont << str.trimmed();
            } //endif
        } //endwhile

        if (fcont.count() == 1) {
            return true;
        } else {
            return false;
        } //endif
    } else {
        return false;
    } //endif
} //endfunction keyFileValid

void IOHelper::writeNewKey(QString key) {
    QFile keyFile(IOHelper::dirpath + IOHelper::keyname);
    if (keyFile.exists()) {
        if (!keyFile.remove()) {
            qDebug("ERROR: Keyfile has missing read/write permission.");
            exit(0);
        } //endif
    } //endif

    if (keyFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&keyFile);
        out << key.toStdString().c_str() << endl;
        keyFile.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ReadGroup|QFile::WriteGroup);
        keyFile.close();
    } else {
        qDebug("ERROR: Keyfile has missing read/write permissions.");
        exit(0);
    } //endif
} //endfunction writeNewKey

QCA::SecureArray IOHelper::readKey() {
    if (!IOHelper::keyFileValid()) {
        CryptoHelper *crypto = new CryptoHelper("aes", 256);
        IOHelper::writeNewKey(crypto->generateKey());
    } //endif

    QFile keyFile(IOHelper::dirpath + IOHelper::keyname);
    keyFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QStringList fcont;
    QTextStream in(&keyFile);
    while (!in.atEnd()) {
        QString str = in.readLine();
        if (str.length() > 0) {
            fcont << str.trimmed();
        } //endif
    } //endwhile

    QCA::SecureArray retVal = fcont.at(0).toAscii();
    return retVal;
} //endfunction readKey
