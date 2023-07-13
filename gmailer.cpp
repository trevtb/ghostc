 #include "gmailer.h"

GMailer::GMailer(QObject *parent) : QObject(parent) {
} //endconstructor

GMailer::GMailer(QMap<QString, QString> *settings, QObject *parent) {
    this->settings = settings;
} //endconstructor

bool GMailer::sendEmails() {
    CryptoHelper *crypt_ref = new CryptoHelper("aes", 256, this);
    crypt_ref->setKey(IOHelper::readKey());
    QString keyval = crypt_ref->getNewAPIKey();

    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    if (connected) {
        dbhelper->updateKey(keyval);
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;

    QString url = crypt_ref->decrypt(settings->value("ghostphp_host"));

    qDebug("INFO: Generated new API key and updated database accordingly.");

    SmtpHelper *helper_ref = new SmtpHelper(settings, this);
    bool success = helper_ref->sendMails(url + "/?apikey=" + keyval);

    return success;
} //endfunction sendEmails

bool GMailer::notify(QStringList recipients, QString subject, QString text, bool silent=false) {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();

    QStringList mailaddrs;
    if (connected) {
        for (int i=0; i<recipients.count(); i++) {
            QList<QString> recpinfo = dbhelper->getDecryptedUser(recipients.at(i));
            if (sizeof(recpinfo) == 0) {
                return false;
            } //endif
            mailaddrs << recpinfo.at(1);
        } //endfor

        SmtpHelper *smtp = new SmtpHelper(settings, this);
        bool success = smtp->sendNotify(mailaddrs, subject, text, silent);

        return success;
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif
} //endfunction notify
