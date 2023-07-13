#include "dbhelper.h"

DbHelper::DbHelper(QObject *parent) : QObject(parent) {
} //endconstructor

DbHelper::DbHelper(QMap<QString, QString> *settings, QObject *parent) {
    this->crypt = new CryptoHelper("aes", 256, this);
    this->crypt->setKey(IOHelper::readKey());

    this->dbhost = settings->value("db_host");
    this->dbport = settings->value("db_port");
    this->dbname = settings->value("db_name");
    this->dbuser = settings->value("db_user");
    this->dbpass = settings->value("db_pass");

    dbstruct.insert("chatmessage", (QStringList() << "id" << "reqid" << "user" << "recipient" << "time" << "message" << "type"));
    dbstruct.insert("key", (QStringList() << "id" << "value"));
    dbstruct.insert("mailaccount", (QStringList() << "id" << "host" << "port" << "auth" << "ssl_tls" << "username" << "password" << "email" << "max_recipients" << "send_delay" << "count"));
    dbstruct.insert("request", (QStringList() << "id" << "user" << "recipient" << "time" << "ucount" << "rcount" << "utime" << "rtime"));
    dbstruct.insert("user", (QStringList() << "id" << "email" << "username" << "password"));

    if (!supportsMySQL()) {
        qDebug("ERROR: mySQL is not supported - please check if libqt4-sql-mysql is installed.");
        exit(0);
    } //endif
} //endconstructor

bool DbHelper::supportsMySQL() {
    bool retVal = false;
    QStringList drivers = QSqlDatabase::drivers();
    foreach (QString driver, drivers) {
        if (QString::compare(driver, "QMYSQL") == 0) {
            retVal = true;
        } //endif
    } //endforeach

    return retVal;
} //endfunction supportsMySQL

bool DbHelper::connectToDb() {
    db = QSqlDatabase::database();
    if (db.isValid()) {
        return true;
    } //endif

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(crypt->decrypt(dbhost));
    db.setPort(QString(crypt->decrypt(dbport)).toInt());
    db.setDatabaseName(crypt->decrypt(dbname));
    db.setUserName(crypt->decrypt(dbuser));
    db.setPassword(crypt->decrypt(dbpass));

    if (!db.open()) {
        return false;
    } //endif

   return true;
} //endfunction connectToDb

void DbHelper::closeDbConnection() {
    this->db.close();
    QSqlDatabase::removeDatabase("QMYSQL");
} //endfunction closeDbConnection

QList<QList<QString> > DbHelper::getTable(QString name) {
    if (!isValidTable(name)) {
        qDebug("ERROR: Internal error occured, tried to access not existing table.");
        exit(0);
    } //endif

    QList<QList<QString> > table;
    QSqlQuery query(QString("SELECT * FROM `" + name + "`;"));

    if (query.isActive()) {
        while (query.next()) {
            QList<QString> row;
            for (int i=0; i<query.record().count(); i++) {
                row.append(query.value(i).toString());
            } //endfor

            table.append(row);
        } //endwhile
    } //endif

    return table;
} //endfunction getTable


QList<QList<QString> > DbHelper::getDecryptedTable(QString tabname) {
    Validator *val = new Validator();
    QList<QList<QString> > tab = getTable(tabname);
    QList<QList<QString> > dectab;
    foreach (QList<QString> row, tab) {
        QList<QString> decrow;
        for (int i=0; i<row.count(); i++) {
            if (val->isEncryptedValue(row.at(i))) {
                decrow.append(crypt->decrypt(row.at(i)));
            } else {
                decrow.append(row.at(i));
            } //endif
        } //endforeach
        dectab.append(decrow);
    } //endforeach

    return dectab;
} //endfunction getDecryptedTable

bool DbHelper::isValidTable(QString tname) {
    if (dbstruct.contains(tname)) {
        return true;
    } else {
        return false;
    } //endif
} //endfunction isValidTable

bool DbHelper::isValidAttribute(QString tablename, QString columnname) {
    QStringList row = dbstruct.value(tablename);
    bool retVal = false;
    foreach (QString entry, row) {
        if (QString::compare(entry, columnname) == 0) {
            retVal = true;
        } //endif
    } //endforeach

    return retVal;
} //endfunction isValidAttribute

void DbHelper::updateKey(QString key) {
    QList<QList<QString> > keytab = getDecryptedTable("key");
    int results = keytab.count();

    QSqlQuery qtwo;
    if (results == 1) {
        QString id = keytab.at(0).at(0);
        qtwo.prepare("UPDATE `key` SET `value`=:key WHERE `id`=:id;");
        qtwo.bindValue(":key", crypt->encrypt(key));
        qtwo.bindValue(":id", id);
        qtwo.exec();
    } else if (results == 0) {
        qtwo.prepare("INSERT INTO `key` (`id`, `value`) VALUES (NULL, :key);");
        qtwo.bindValue(":key", crypt->encrypt(key));
        qtwo.exec();
    } else {
        qtwo.exec("TRUNCATE TABLE `key`;");
        qtwo.prepare("INSERT INTO `key` (`id`, `value`) VALUES (NULL, :key);");
        qtwo.bindValue(":key", crypt->encrypt(key));
        qtwo.exec();
    } //endif
} //endfunction updateKey

void DbHelper::incrementAccountCounter(int id) {
    QSqlQuery query;
    query.prepare("UPDATE `mailaccount` SET `count`=`count`+1 WHERE `id`=:id;");
    query.bindValue(":id", id);
    query.exec();
} //endfunction incrementAccountCounter

void DbHelper::addUser(QString mail, QString user, QString pass) {
    if (userExists(user, mail)) {
        qDebug("ERROR: User with given email address or username already exists.");
    } else {
        qDebug("INFO: Adding user >>> %s <<<", user.toStdString().c_str());
        QSqlQuery q;
        q.prepare("INSERT INTO `user` (`id`, `email`, `username`, `password`) VALUES (NULL, :mail, :user, :pass);");
        q.bindValue(":mail", crypt->encrypt(mail));
        q.bindValue(":user", crypt->encrypt(user));
        q.bindValue(":pass", crypt->encrypt(crypt->getPasswordHash(pass)));
        q.exec();
        qDebug("INFO: Done.");
    } //endif
} //endfunction addUser

void DbHelper::delUser(int id) {
    QSqlQuery query;
    query.prepare("SELECT * FROM `user` WHERE `id`=:id;");
    query.bindValue(":id", id);
    if (query.exec() && query.next()) {
        qDebug("INFO: Deleting user >>> %s <<<", crypt->decrypt(query.value(2).toString()).toStdString().c_str());
        QSqlQuery q;
        q.prepare("DELETE FROM `user` WHERE `id`=:id");
        q.bindValue(":id", id);
        q.exec();
        qDebug("INFO: Done.");
    } else {
        qDebug("ERROR: User does not exist.");
    } //endif
} //endfunction delUser

void DbHelper::addMailAccount(QString host, int port, int auth, int ssl_tls, QString user,
                           QString pass, QString email, int max_recipients, int delay) {
    if (mailAccountExists(email)) {
        qDebug("ERROR: Account with given email address already exists.");
    } else {
        qDebug("INFO: Adding new account with the following parameters:");
        qDebug("Host:\t\t%s", host.toStdString().c_str());
        qDebug("Port:\t\t%i", port);
        if (auth == 0) {
            qDebug("Authentication:\tNo");
        } else if (auth == 1) {
            qDebug("Authentication:\tYes");
        } //endif
        if (ssl_tls == 0) {
            qDebug("Encryption:\tNone");
        } else if (ssl_tls == 1) {
            qDebug("Encryption:\tSSL");
        } else if (ssl_tls == 2) {
            qDebug("Encryption:\tTLS");
        } //endif
        qDebug("Username:\t%s", user.toStdString().c_str());
        qDebug("E-Mail:\t\t%s", email.toStdString().c_str());
        qDebug("Max Recipients:\t%i", max_recipients);
        qDebug("Delay:\t\t%i", delay);

        QSqlQuery q;
        QString qstring = "INSERT INTO `mailaccount` (`host`, `port`, `auth`, `ssl_tls`, `username`, "
                            "`password`, `email`, `max_recipients`, `send_delay`, `count`) "
                            "VALUES (:host, :port, :auth, :ssl_tls, :username, :password, :email, "
                            ":max_recipients, :send_delay, :count);";
        q.prepare(qstring);
        q.bindValue(":host", crypt->encrypt(host));
        q.bindValue(":port", QString::number(port));
        q.bindValue(":auth", QString::number(auth));
        q.bindValue(":ssl_tls", QString::number(ssl_tls));
        q.bindValue(":username", crypt->encrypt(user));
        q.bindValue(":password", crypt->encrypt(pass));
        q.bindValue(":email", crypt->encrypt(email));
        q.bindValue(":max_recipients", QString::number(max_recipients));
        q.bindValue(":send_delay", QString::number(delay));
        q.bindValue(":count", "0");
        q.exec();
        qDebug("INFO: Done.");
    } //endif
} //endfunction addMailAccount

void DbHelper::delMailAccount(int id) {
    QSqlQuery query;
    query.prepare("SELECT * FROM `mailaccount` WHERE `id`=:id;");
    query.bindValue(":id", id);
    if (query.exec() && query.next()) {
        qDebug("INFO: Deleting mail account with email address >>> %s <<<", crypt->decrypt(query.value(7).toString()).toStdString().c_str());
        QSqlQuery q;
        q.prepare("DELETE FROM `mailaccount` WHERE `id`=:id;");
        q.bindValue(":id", id);
        q.exec();
        qDebug("INFO: Done.");
    } else {
        qDebug("ERROR: Account does not exist.");
    } //endif
} //endfunction delMailAccount

void DbHelper::update(QString table, int id, QString attribute, QString value) {
    if (isValidTable(table) && isValidAttribute(table, attribute)) {
        bool isencval = false;
        Validator *val = new Validator();
        QSqlQuery q;
        QString qs = "SELECT `" + attribute + "` FROM `" + table + "` WHERE `id`=:id;";
        q.prepare(qs);
        q.bindValue(":id", id);
        q.exec();
        if (q.next()) {
            if (val->isEncryptedValue(q.value(0).toString())) {
                isencval = true;
            } //endif
        } else {
            qDebug("ERROR: Could not read from database.");
        } //endif

        QSqlQuery query;
        query.prepare(QString("SELECT * FROM `" + table + "` WHERE `id`=:id;"));
        query.bindValue(":id", id);
        if (query.exec() && query.next()) {
            if (isencval) {
                value = crypt->encrypt(value);
            } //endif
            QString qstri = QString("UPDATE `" + table + "` SET `" + attribute + "`=:value WHERE `id`=:id;");
            QSqlQuery q;
            q.prepare(qstri);
            q.bindValue(":value", value);
            q.bindValue(":id", id);
            q.exec();
        } else {
            qDebug("ERROR: Entry with id #%i does not exist.", id);
        } //endif
    } else {
        qDebug("ERROR: Invalid table or attribute name provided to update function.");
    } //endif
} //endfunction update

void DbHelper::doInstall() {
    QStringList querycmds;
    querycmds << "SET SQL_MODE = \"NO_AUTO_VALUE_ON_ZERO\";";
    querycmds << "SET AUTOCOMMIT = 0;";
    querycmds << "START TRANSACTION;";
    querycmds << "SET time_zone = \"+00:00\";";
    querycmds << "USE :dbname;";
    querycmds << "CREATE TABLE IF NOT EXISTS `key` ("
                 "`id` int(9) NOT NULL AUTO_INCREMENT,"
                 "`value` varchar(512) NOT NULL,"
                 "PRIMARY KEY (`id`)"
                 ") ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;";
    querycmds << "CREATE TABLE IF NOT EXISTS `request` ("
                 "`id` int(9) NOT NULL AUTO_INCREMENT,"
                 "`user` int(9) NOT NULL,"
                 "`recipient` int(9) NOT NULL,"
                 "`time` varchar(32) NOT NULL,"
                 "`ucount` int(9) NOT NULL DEFAULT '0',"
                 "`rcount` int(9) NOT NULL DEFAULT '0',"
                 "`utime` varchar(32) NOT NULL DEFAULT '0',"
                 "`rtime` varchar(32) NOT NULL DEFAULT '0',"
                 "`password` varchar(512) NOT NULL DEFAUL '0',"
                 "PRIMARY KEY (`id`)"
                 ") ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;";
    querycmds << "CREATE TABLE IF NOT EXISTS `chatmessage` ("
                 "`id` int(9) NOT NULL AUTO_INCREMENT,"
                 "`reqid` int(9) NOT NULL,"
                 "`user` int(9) NOT NULL,"
                 "`recipient` int(9) NOT NULL,"
                 "`time` varchar(32) NOT NULL,"
                 "`message` text NOT NULL,"
                 "`type` int(9) NOT NULL DEFAULT '0',"
                 "PRIMARY KEY (`id`)"
                 ") ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;";
    querycmds << "CREATE TABLE IF NOT EXISTS `mailaccount` ("
                 "`id` int(9) NOT NULL AUTO_INCREMENT,"
                 "`host` varchar(512) NOT NULL,"
                 "`port` int(9) NOT NULL,"
                 "`auth` int(9) NOT NULL,"
                 "`ssl_tls` int(9) NOT NULL,"
                 "`username` varchar(512) NOT NULL,"
                 "`password` varchar(512) NOT NULL,"
                 "`email` varchar(512) NOT NULL,"
                 "`max_recipients` int(9) NOT NULL DEFAULT '10',"
                 "`send_delay` int(9) NOT NULL DEFAULT '5000',"
                 "`count` int(9) NOT NULL DEFAULT '0',"
                 "PRIMARY KEY (`id`)"
                 ") ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;";
    querycmds << "CREATE TABLE IF NOT EXISTS `user` ("
                 "`id` int(9) NOT NULL AUTO_INCREMENT,"
                 "`email` varchar(512) NOT NULL,"
                 "`username` varchar(512) NOT NULL,"
                 "`password` varchar(512) NOT NULL,"
                 "PRIMARY KEY (`id`),"
                 "UNIQUE KEY `email` (`email`,`username`)"
                 ") ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;";
    querycmds << "COMMIT;";

    foreach (QString qstr, querycmds) {
        QSqlQuery query;
        query.prepare(qstr);
        if(qstr.contains(":dbname")) {
            query.bindValue(":dbname", crypt->decrypt(dbname));
        } //endif

        query.exec();
    } //endforeach
} //endfunction doInstall

bool DbHelper::userExists(QString username, QString email) {
    QList<QList<QString> > usertable = getTable("user");
    bool exists = false;
    foreach (QList<QString> row, usertable) {
        QString mail = crypt->decrypt(row.at(1));
        QString user = crypt->decrypt(row.at(2));
        if (QString::compare(mail, email) == 0 ||
            QString::compare(user, username) == 0) {
            exists = true;
        } //endif
    } //endforeach

    return exists;
} //endfunction userExists

bool DbHelper::userExists(QString username) {
    QList<QList<QString> > usertable = getTable("user");
    bool exists = false;
    foreach (QList<QString> row, usertable) {
        QString user = crypt->decrypt(row.at(2));
        if (QString::compare(user, username) == 0) {
            exists = true;
        } //endif
    } //endforeach

    return exists;
} //endfunction userExists

bool DbHelper::mailAccountExists(QString mailaddr) {
    QList<QList<QString> > acctable = getTable("mailaccount");

    bool exists = false;
    foreach (QList<QString> row, acctable) {
        QString mail = crypt->decrypt(row.at(7));
        if (QString::compare(mail, mailaddr) == 0) {
            exists = true;
        } //endif
    } //endforeach

    return exists;
} //endfunction mailAccountExists

QString DbHelper::reencrypt() {
    Validator *val = new Validator();
    CryptoHelper *newcrypt = new CryptoHelper("aes", 256, this);
    QString newkey = newcrypt->generateKey();
    QCA::SecureArray secar = newkey.toAscii();
    newcrypt->setKey(secar);

    QMap<QString, QStringList>::Iterator iter;
    iter = dbstruct.begin();
    while (iter != dbstruct.end()) {
        QList<QList<QString> > enctab = getTable(iter.key());

        if (enctab.count() < 1) {
            qDebug("INFO: Table %s is empty, skipping.", QString(iter.key()).toStdString().c_str());
        } else {
            QList<QList<QString> > enctabnew;
            foreach (QList<QString> encrow, enctab) {
                QList<QString> encrownew;
                for (int i=0; i<encrow.count(); i++) {
                    QString encvalnew;
                    if (val->isEncryptedValue(encrow.at(i))) {
                        encvalnew = newcrypt->encrypt(crypt->decrypt(encrow.at(i)));
                    } else {
                        encvalnew = encrow.at(i);
                    } //endif
                    encrownew.append(encvalnew);
                } //endfor
                enctabnew.append(encrownew);
            } //endforeach

            QSqlQuery q1;
            q1.exec(QString("TRUNCATE TABLE `" + iter.key() + "`;"));

            QStringList columns = dbstruct.value(iter.key());
            foreach (QList<QString> row, enctabnew) {
                QString qval = QString("INSERT INTO `" + iter.key() + "` (");
                for (int i=0; i<columns.count(); i++) {
                    qval += QString("`" + columns.at(i) + "`");
                    if (i < (columns.count() - 1)) {
                        qval += QString(", ");
                    } //endif
                } //endfor

                qval += QString(") VALUES (");

                for (int i=0; i<columns.count(); i++) {
                    qval += QString(":" + columns.at(i));
                    if (i < (columns.count() - 1)) {
                        qval += QString(", ");
                    } //endif
                } //endfor

                qval += QString(");");

                QSqlQuery q2;
                q2.prepare(qval);

                for (int i=0; i<columns.count(); i++) {
                    QString coln = QString(":" + columns.at(i));
                    QString val = row.at(i);
                    q2.bindValue(coln, val);
                } //endfor

                q2.exec();
            } //endforeach
        } //endif

        iter++;
    } //endwhile

    return newkey;
} //endfunction newkey

QList<QString> DbHelper::getDecryptedUser(QString username) {
    QList<QList<QString> > usertab = getDecryptedTable("user");

    QList<QString> retVal;
    foreach (QList<QString> usr, usertab) {
        if (QString::compare(usr.at(2), username) == 0) {
            retVal = usr;
        } //endif
    } //endforeach

    return retVal;
} //endfunction getDecryptedUser

QList<QString> DbHelper::getDecryptedUser(int id) {
    QList<QString> retVal;

    Validator *val = new Validator();
    QSqlQuery q;
    q.prepare("SELECT * FROM `user` WHERE `id`=:id");
    q.bindValue(":id", id);
    if (q.exec() && q.next()) {
        for (int i=0; i<q.record().count(); i++) {
            if (val->isEncryptedValue(q.value(i).toString())) {
                retVal.append(crypt->decrypt(q.value(i).toString()));
            } else {
                retVal.append(q.value(i).toString());
            } //endif
        } //endfor
    } //endif

    return retVal;
} //endfunction getDecryptedUser

QList<QList<QString> > DbHelper::getUserRequests(QString username) {
    Validator *validator = new Validator();
    QList<QString> user = getDecryptedUser(username);
    int id = user.at(0).toInt();

    QList<QList<QString> > requests;
    QSqlQuery query;
    query.prepare("SELECT * FROM `request` WHERE `user`=:uid OR `recipient`=:rid;");
    query.bindValue(":uid", id);
    query.bindValue(":rid", id);
    query.exec();

    while (query.next()) {
        QList<QString> row;
        for (int i=0; i<query.record().count(); i++) {
            QString val = query.value(i).toString();

            if (i == 1 || i == 2) {
                QSqlQuery q;
                q.prepare("SELECT * FROM `user` WHERE `id`=:id");
                q.bindValue(":id", val.toInt());
                if (q.exec() && q.next()) {
                    QString un = q.value(2).toString();
                    row.append(crypt->decrypt(un));
                } //endif
            } else if (i == 8 && validator->isEncryptedValue(val)) {
                row.append(crypt->decrypt(val));
            } else {
                row.append(val);
            } //endif
        } //endfor

        requests.append(row);
    } //endwhile

    return requests;
} //endfunction getUserRequests

QList<QString> DbHelper::getUserRequest(int reqid) {
    Validator *validator = new Validator();
    QSqlQuery q;
    q.prepare("SELECT * FROM `request` WHERE `id`=:reqid;");
    q.bindValue(":reqid", reqid);
    q.exec();

    QList<QString> row;
    if (q.next()) {
       for (int i=0; i<q.record().count(); i++) {
            if (i == 8 && validator->isEncryptedValue(q.value(i).toString())) {
                row.append(crypt->decrypt(q.value(i).toString()));
            } else {
                row.append(q.value(i).toString());
            } //endif
        } //endfor
    } //endif

    return row;
} //endfunction getUserRequest

QString DbHelper::writeNewRequest(QString user, QString recipient, QString pass) {
    QList<QString> usr = getDecryptedUser(user);
    int uid = usr.at(0).toInt();

    QList<QString> rcp = getDecryptedUser(recipient);
    int rid = rcp.at(0).toInt();

    QSqlQuery q;
    q.prepare("INSERT INTO `request` (`user`, `recipient`, `time`, `ucount`, `rcount`, `utime`, `rtime`, `password`) VALUES (:user, :recp, :time, :ucount, :rcount, :utime, :rtime, :password);");
    q.bindValue(":user", uid);
    q.bindValue(":recp", rid);
    q.bindValue(":time", QString::number(time(0)));
    q.bindValue(":ucount", 0);
    q.bindValue(":rcount", 0);
    q.bindValue(":utime", 0);
    q.bindValue(":rtime", 0);
    q.bindValue(":password", crypt->encrypt(pass));
    q.exec();

    QString id = "-1";
    QSqlQuery query;
    query.prepare("SELECT LAST_INSERT_ID();");
    if (query.exec() && query.next()) {
        id = query.value(0).toString();
    } //endif

    return id;
} //endfunction writeNewRequest

void DbHelper::deleteRequest(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM `request` WHERE `id`=:id;");
    query.bindValue(":id", id);
    query.exec();
} //endfunction deleteRequest

void DbHelper::enterChat(QString user, int id, QString type) {
    QStringList fields;
    if (QString::compare(type, "user") == 0) {
        fields << "ucount";
        fields << "utime";
    } else {
        fields << "rcount";
        fields << "rtime";
    } //endif

    QSqlQuery q;
    QString qstri = "UPDATE `request` SET ";
    qstri.append("`"+fields.at(0)+"`=`"+fields.at(0)+"`+1, ");
    qstri.append("`"+fields.at(1)+"`=:newt WHERE `id`=:id;");
    q.prepare(qstri);
    q.bindValue(":newt", QString::number(time(0)));
    q.bindValue(":id", id);
    q.exec();
} //endfunction enterChat

QList<QString> DbHelper::getRequestProtectParams(QString user, int reqid) {
    QList<QList<QString> > reqs = getUserRequests(user);
    QList<QString> req;
    for (int i=0; i<reqs.count(); i++) {
        if (QString::compare(reqs.at(i).at(0), QString::number(reqid)) == 0) {
            req = reqs.at(i);
        } //endif
    } //endfor

    QString reqtime;
    QString count;
    QString type;

    QList<QString> usrar = getDecryptedUser(user);
    if (QString::compare(usrar.at(2), req.at(1)) == 0) {
        reqtime = req.at(6);
        count = req.at(4);
        type = "user";
    } else {
        reqtime = req.at(7);
        count = req.at(5);
        type = "recipient";
    } //endif

    QList<QString> retAr;
    retAr.append(reqtime);
    retAr.append(count);
    retAr.append(type);

    return retAr;
} //endfunction getRequestProtectParams

void DbHelper::addChatMessage(int reqid, int user, int recipient, int time, QString message, int type) {
    QSqlQuery q;
    QString qstr = "INSERT INTO `chatmessage` (`reqid`, `user`, `recipient`, `time`, `message`, `type`) VALUES (:reqid, :user, :recipient, :time, :message, :type);";
    q.prepare(qstr);
    q.bindValue(":reqid", reqid);
    q.bindValue(":user", user);
    q.bindValue(":recipient", recipient);
    q.bindValue(":time", time);
    q.bindValue(":message", crypt->encrypt(message));
    q.bindValue(":type", type);
    q.exec();
} //endfunction addChatMessage

QList<QList<QString> > DbHelper::getDecryptedChatMessages(int reqid) {
    Validator *val = new Validator();
    QList<QList<QString> > messages;
    QSqlQuery q;
    q.prepare("SELECT * FROM `chatmessage` WHERE `reqid`=:reqid ORDER BY `time` ASC;");
    q.bindValue(":reqid", reqid);
    q.exec();

    while (q.next()) {
        QList<QString> row;
        for (int i=0; i<q.record().count(); i++) {
            if (val->isEncryptedValue(q.value(i).toString())) {
                row.append(crypt->decrypt(q.value(i).toString()));
            } else {
                row.append((q.value(i).toString()));
            } //endif
        } //endfor

        messages.append(row);
    } //endwhile

    return messages;
} //endfunction getDecryptedChatMessages

void DbHelper::cleanChatHistory(int reqid) {
    QList<QList<QString> > messages = getDecryptedChatMessages(reqid);
    QList<QList<QString> > messagesNEW;

    int max = 20;
    if (messages.count() < 20) {
        max = messages.count();
    } //endif

    int count = 0;
    while (count < max) {
        messagesNEW.append(messages.at(messages.count() - 1 - count));
        count++;
    } //endwhile

    QSqlQuery q1;
    q1.prepare("DELETE FROM `chatmessage` WHERE `reqid`=:reqid;");
    q1.bindValue(":reqid", reqid);
    q1.exec();

    foreach (QList<QString> msg, messagesNEW) {
        QSqlQuery q;
        q.prepare("INSERT INTO `chatmessage` (`reqid`, `user`, `recipient`, `time`, `message`, `type`) VALUES (:reqid, :user, :recipient, :time, :message, :type);");
        q.bindValue(":reqid", reqid);
        q.bindValue(":user", msg.at(2).toInt());
        q.bindValue(":recipient", msg.at(3).toInt());
        q.bindValue(":time", msg.at(4));
        q.bindValue(":message", crypt->encrypt(msg.at(5)));
        q.bindValue(":type", msg.at(6).toInt());
        q.exec();
    } //endforeach
} //endfunction cleanChatHistory

void DbHelper::deleteChat(int reqid) {
    QSqlQuery q;
    q.prepare("DELETE FROM `chatmessage` WHERE `reqid`=:reqid;");
    q.bindValue(":reqid", reqid);
    q.exec();
} //endfunction deleteChat

void DbHelper::nullAccountCounters() {
    QList<QList<QString> > accounttab = getTable("mailaccount");
    foreach (QList<QString> account, accounttab) {
        QSqlQuery q;
        QString qstri = "UPDATE `mailaccount` SET `count`=:count WHERE `id`=:id;";
        q.prepare(qstri);
        q.bindValue(":count", 0);
        q.bindValue(":id", account.at(0).toInt());
        q.exec();
    } //endforeach
} //endfunction nullAccountCounters

QString DbHelper::getRequestPassword(int id) {
    QString pass;
    QList<QString> req = getUserRequest(id);
    if (req.count() > 0) {
        pass = req.at(8);
    } else {
        pass = "0";
    } //endif

    return pass;
} //endfunction getRequestPassword
