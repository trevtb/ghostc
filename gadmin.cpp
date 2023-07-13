#include "gadmin.h"

GAdmin::GAdmin(QObject *parent) : QObject(parent) {
} //endconstructor

GAdmin::GAdmin(QMap<QString, QString> *settings, QObject *parent) : QObject(parent) {
    this->settings = settings;
} //endconstructor

void GAdmin::printUserInfo() {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    if (connected) {
        qDebug("########## USER INFORMATION ##########");
        QList<QList<QString> > user_data = dbhelper->getDecryptedTable("user");
        if (user_data.count() < 1) {
            qDebug("< User database is empty. >");
        } //endif

        QList<QString> row;
        foreach(row, user_data) {
            qDebug("\nID:\t\t%s", row[0].toStdString().c_str());
            qDebug("E-Mail:\t\t%s", row[1].toStdString().c_str());
            qDebug("Username:\t%s", row[2].toStdString().c_str());
        } //endforeach
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction printUserInfo

void GAdmin::printAPIKey() {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    if (connected) {
        QList<QList<QString> > keytab = dbhelper->getDecryptedTable("key");
        if (keytab.count()  == 1) {
            QList<QString> row = keytab.at(0);
            QString key = row.at(1);
            qDebug(key.toStdString().c_str());
        } else if (keytab.count() == 0){
            qDebug("INFO: Key table is empty.");
            exit(0);
        } else {
            qDebug("ERROR: Key table contains multiple entries, you will have to fix this.");
        } //endif
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction printAPIKey

void GAdmin::printAccountInfo() {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    if (connected) {
        qDebug("########## ACCOUNT INFORMATION ##########");
        QList<QList<QString> > account_data = dbhelper->getDecryptedTable("mailaccount");
        if (account_data.count() < 1) {
            qDebug("< Account database is empty. >");
        } //endif

        QList<QString> row;
        foreach(row, account_data) {
            qDebug("\nID:\t\t%s", row[0].toStdString().c_str());
            qDebug("Host:\t\t%s", row[1].toStdString().c_str());
            qDebug("Port:\t\t%s", row[2].toStdString().c_str());

            if (QString::compare(row[3], "1") == 0) {
                qDebug("Auth:\t\tYes");
            } else {
                qDebug("Auth:\t\tNo");
            } //endif

            if (QString::compare(row[4], "0") == 0) {
                qDebug("SSL/TLS:\tNo");
            } else if (QString::compare(row[4], "1") == 0) {
                qDebug("SSL/TLS:\tSSL");
            } else if (QString::compare(row[4], "2") == 0) {
                qDebug("SSL/TLS:\tTLS");
            } //endif
            qDebug("Username:\t%s", row[5].toStdString().c_str());
            qDebug("E-Mail:\t\t%s", row[7].toStdString().c_str());
            qDebug("Max Recipients:\t%s", row[8].toStdString().c_str());
            qDebug("Send Delay:\t%s ms\n", row[9].toStdString().c_str());
        } //endforeach
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction printAccountInfo

void GAdmin::printOpenRequests(QString username) {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    if (connected) {
        QString outval = "";
        QList<QList<QString> > requests = dbhelper->getUserRequests(username);
        if (requests.count() > 0) {
            for (int i=0; i<requests.count(); i++) {
                for (int j=0; j<requests.at(i).count()-1; j++) {
                    outval += requests.at(i).at(j);

                    if (j < (requests.at(i).count() - 2)) {
                        outval += ",";
                    } //endif
                } //endfor

                if (i < (requests.count() - 1)) {
                    outval += ";";
                } //endif
            } //endfor

            qDebug(outval.toStdString().c_str());
        } else {
            qDebug("0");
        } //endif
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction printOpenRequests

void GAdmin::addUser(QString mail, QString user, QString pass) {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    if (connected) {
        dbhelper->addUser(mail, user, pass);
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction addUser

void GAdmin::delUser(int id) {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    if (connected) {
        dbhelper->delUser(id);
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction delUser

void GAdmin::updateUser(int id, QString attribute, QString value) {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    if (connected) {
        dbhelper->update("user", id, attribute, value);
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction updateUser

void GAdmin::addMailAccount(QString host, QString port, QString auth, QString ssl_tls, QString user,
                           QString pass, QString email, QString max_recipients, QString delay) {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    if (connected) {
        dbhelper->addMailAccount(host, port.toInt(), auth.toInt(), ssl_tls.toInt(), user, pass, email, max_recipients.toInt(), delay.toInt());
        dbhelper->nullAccountCounters();
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction addMailAccount

void GAdmin::delMailAccount(int id) {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    if (connected) {
        dbhelper->delMailAccount(id);
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction delMailAccount

void GAdmin::updateAccount(int id, QString attribute, QString value) {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    if (connected) {
        dbhelper->update("mailaccount", id, attribute, value);
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction updateAccount

void GAdmin::doInstall() {
    qDebug("INFO: Creating database tables. Will not overwrite.");

    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();

    if (connected) {
        dbhelper->doInstall();
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
    qDebug("INFO: Creation finished successfully.");
} //endfunction doInstall

void GAdmin::reencrypt() {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();

    if (connected) {
        QString newkey = dbhelper->reencrypt();
        IOHelper *ioh = new IOHelper(this);
        ioh->writeNewKey(newkey);
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
    qDebug("INFO: Re-encryption finished successfully.");
} //endfunction reencrypt

void GAdmin::checkLogin(QString user, QString pwdhash) {
    bool valid = false;
    CryptoHelper *crypt = new CryptoHelper("aes", 256);
    crypt->setKey(IOHelper::readKey());
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();

    if (connected) {
        QList<QList<QString> > usertable = dbhelper->getDecryptedTable("user");
        foreach (QList<QString> row, usertable) {
            if (QString::compare(user, row.at(2)) == 0 &&
                QString::compare(pwdhash, row.at(3)) == 0) {
                valid = true;
            } //endif
        } //endforeach

        if (valid) {
            QString timeout = crypt->decrypt(settings->value("request_timeout"));
            qDebug(timeout.toStdString().c_str());
        } else {
            qDebug("false");
        } //endif
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction checkLogin

QString GAdmin::writeNewRequest(QString username, QString recipient, QString pass) {
    QString retVal = "-1";
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();

    if (connected) {
        retVal = dbhelper->writeNewRequest(username, recipient, pass);
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;

    return retVal;
} //endfunction writeNewRequest

bool GAdmin::userExists(QString username) {
    bool retVal = false;
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();

    if (connected) {
        retVal = dbhelper->userExists(username);
    } else {
        qDebug("ERROR: Could not connect to database.");
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;

    return retVal;
} //endfunction userExists

void GAdmin::changeSettings(QString user, QString mail, QString pass) {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();

    if (connected) {
        QList<QString> userrow = dbhelper->getDecryptedUser(user);
        if (QString::compare(mail, "empty") != 0) {
            dbhelper->update("user", userrow.at(0).toInt(), "email", mail);
        } //endif

        if (QString::compare(pass, "empty") != 0) {
            dbhelper->update("user", userrow.at(0).toInt(), "password", pass);
        } //endif

        qDebug("1");
    } else {
        qDebug("ERROR: Could not connect to database.");
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction changeSettings

void GAdmin::enterChat(QString user, QString idstring, QString type) {
    int id = idstring.toInt();
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    int ty = 1;

    if (connected) {
        dbhelper->enterChat(user, id, type);
        QString message;
        message.append("User >> ");
        message.append(user);
        message.append(" << has entered the chat.");
        addChatMessage(idstring.toInt(), user, message, ty, true);
        qDebug("1");
    } else {
        qDebug("ERROR: Could not connect to database.");
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction enterChat

void GAdmin::addChatMessage(int reqid, QString username, QString message, int type, bool silent) {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();

    if (connected) {
        QList<QString> user = dbhelper->getDecryptedUser(username);
        QList<QString> request = dbhelper->getUserRequest(reqid);
        int userid;
        int recpid;
        if (QString::compare(user.at(0), request.at(1)) == 0) {
            userid = request.at(1).toInt();
            recpid = request.at(2).toInt();
        } else if (QString::compare(user.at(0), request.at(2)) == 0) {
            userid = request.at(2).toInt();
            recpid = request.at(1).toInt();
        } else {
            qDebug("ERROR: Permission denied.");
            exit(0);
        } //endif

        int ctime = time(0);

        QUrl tempuri = QUrl::fromPercentEncoding(message.toUtf8());
        message = tempuri.toString();
        dbhelper->addChatMessage(reqid, userid, recpid, ctime, message, type);
        dbhelper->cleanChatHistory(reqid);

        if (!silent) {
            printChatMessages(reqid);
        } //endif
    } else {
        qDebug("ERROR: Could not connect to database.");
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction addChatMessage

void GAdmin::printChatMessages(int reqid) {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();

    if (connected) {
        QList<QList<QString> > msgArray = dbhelper->getDecryptedChatMessages(reqid);
        QString outStr;
        for (int i=0; i<msgArray.count(); i++) {
            QString type = msgArray.at(i).at(6);
            int userid = msgArray.at(i).at(2).toInt();
            int recpid = msgArray.at(i).at(3).toInt();
            QList<QString> user = dbhelper->getDecryptedUser(userid);
            QList<QString> recipient = dbhelper->getDecryptedUser(recpid);

            QString username = user.at(2);
            QString recpname = recipient.at(2);
            QString time = msgArray.at(i).at(4);
            QString msg = QString::fromUtf8(QUrl::toPercentEncoding(msgArray.at(i).at(5)));

            outStr.append(username);
            outStr.append(",");
            outStr.append(recpname);
            outStr.append(",");
            outStr.append(time);
            outStr.append(",");
            outStr.append(msg);
            outStr.append(",");
            outStr.append(type);

            if (i < (msgArray.count() - 1)) {
                outStr.append(";");
            } //endif
        } //endfor

        if (outStr.size() == 0) {
            qDebug("0");
        } else {
            std::cout << outStr.toStdString().c_str() << std::endl;
        } //endif
    } else {
        qDebug("ERROR: Could not connect to database.");
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction printChatMessages

bool GAdmin::deleteChat(QString reqid) {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    bool retVal = false;

    if (connected) {
        dbhelper->deleteChat(reqid.toInt());
        dbhelper->deleteRequest(reqid.toInt());
        retVal = true;
    } else {
        qDebug("ERROR: Could not connect to database.");
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;

    return retVal;
} //endfunction deleteChat

void GAdmin::cleanTimeouts() {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();

    if (connected) {
        CryptoHelper *ch = new CryptoHelper("aes", 256, this);
        ch->setKey(IOHelper::readKey());
        QList<QList<QString> > reqs = dbhelper->getTable("request");
        int delcount = 0;
        for (int i=0; i<reqs.count(); i++) {
            QString timestr = reqs.at(i).at(3);
            double timel = timestr.toDouble();
            int ctime = time(0);
            double diff = ctime - timel;
            QString tmoutstr = ch->decrypt(settings->value("request_timeout"));
            double timeout = tmoutstr.toDouble();
            if (diff > timeout) {
                deleteChat(reqs.at(i).at(0));
                delcount++;
            } //endfor
        } //endfor
    } else {
        qDebug("ERROR: Could not connect to database.");
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction cleanTimeouts

QString GAdmin::getCurrentAPIURL() {
    QString url = "";
    CryptoHelper *crypt = new CryptoHelper("aes", 256, this);
    crypt->setKey(IOHelper::readKey());

    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    if (connected) {
        QList<QList<QString> > keytab = dbhelper->getDecryptedTable("key");
        QList<QString> row = keytab.at(0);
        QString key = row.at(1);
        QString host = crypt->decrypt(settings->value("ghostphp_host"));
        url = host + "?apikey=" + key;
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;

    return url;
} //endfunction getCurrentAPIURL

void GAdmin::printRequestPassword(QString id) {
    DbHelper *dbhelper = new DbHelper(settings, this);
    bool connected = dbhelper->connectToDb();
    if (connected) {
        QString pass = dbhelper->getRequestPassword(id.toInt());
        qDebug(pass.toStdString().c_str());
    } else {
        qDebug("ERROR: Could not connect to database.");
        exit(0);
    } //endif

    dbhelper->closeDbConnection();
    delete dbhelper;
} //endfunction printRequestPassword
