#include "ghostc.h"

GhostC::GhostC(QObject *parent) : QObject(parent) {
} //endconstructor

GhostC::GhostC(QStringList *args, QObject *parent) : QObject(parent) {
    this->crypt = new CryptoHelper("aes", 256, this);
    crypt->setKey(IOHelper::readKey());
    this->args = args;
    this->port = crypt->encrypt(QString::number(9050));
    validarg = false;
} //endconstructor

void GhostC::printHelp() {
    qDebug("Valid arguments are:\n");
    qDebug("--help: Display this help.\n");
    qDebug("--install: Creates the database and tables. Database name will be taken from the config file.\n");
    qDebug("--recrypt: Re-encrypts all database content with a new random key.\n");
    qDebug("--auth --user <user> --pass <password_hash>: Check if user data is valid. Returns the request timeout in seconds or false.\n");
    qDebug("--list-apikey: Lists the current api key stored in the database.\n");
    qDebug("--list-users: Lists all users stored in the database.\n");
    qDebug("--list-accounts: Lists all mail accounts stored in the database.\n");
    qDebug("--print-reqpwd --id <id>: Prints the request password for the given request id in cleartext.\n");
    qDebug("--list-openreqs --user <user>: Lists the open chat requests for the given username.\n");
    qDebug("--delreq --id <id>: Deletes the request with the given id. Will also delete all existing chat messages.\n");
    qDebug("--mail: Generates a new API key and mails it to all users using automatic SMTP load balancing.\n");
    qDebug("--initiate --user <user> --recp <recipient>: Initiates a new chat request by notify the user and writing the request to the database.\n");
    qDebug("--adduser --mail <email> --user <username> --pass <password>: Adds a new user to the database.\n");
    qDebug("--deluser <id>: Delete the user corresponding to the given id.\n");
    qDebug("--updateuser --id <id> --attribute <attribute> --value <value>: Update a user attribute.");
    qDebug("\tExample: ghostc --updateuser --id 1 --attribute username --value yourname\n");
    qDebug("--changesettings --user <username> --mail <new_mail> --pass <new_pass>: Quick way to update the user credentials. You MUST supply 'empty' for empty values.\n");
    qDebug("--addaccount --host <host> --port <port> --auth <auth> --encryption <ssl_tls> --user <username> --pass <password> --mail <email> --max_recps <max_recipients> --delay <delay>");
    qDebug("\tauth: Authentication required. Yes=1, No=0.");
    qDebug("\tssl_tls: Encryption. None=0, SSL=1, TLS=2.");
    qDebug("\t[optional] max_recipients: Maximum recipients per mail (to+cc+bcc).");
    qDebug("\t[optional] delay: Time in milliseconds to wait between each mail.\n");
    qDebug("--delaccount <id>: Delete the account corresponding to the given id.\n");
    qDebug("--updateaccount --id <id> --attribute <attribute> --value <value>: Update an account attribute.");
    qDebug("\tExample: ghostc --updateaccount --id 1 --attribute email --value you@domain.com\n");
    qDebug("--enterchat --user <user> --reqid <reqid>: Lets the user enter the chat with the given request.\n");
    qDebug("--addchatmsg --reqid <id> --user <username> --message <text>: Adds a new chat message to the server. After completion this function prints all the chat messages for the request.");
    qDebug("\tATTENTION: The text needs to be URL encoded.");
    qDebug("--getchatmessages --reqid <id>: Get the chat messages for the given request.\n");
    qDebug("--cleantimeouts: Deletes all timed out requests and their corresponding chat messages.\n");
} //endfunction printHelp

bool GhostC::startTor(bool silent=false) {
    IpApiHelper *iphelper = new IpApiHelper(this);
    QString ip = iphelper->getExternalIP();

    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::Socks5Proxy);
    proxy.setHostName("127.0.0.1");
    proxy.setPort(QString(crypt->decrypt(port)).toInt());
    QNetworkProxy::setApplicationProxy(proxy);

    QString ipnew = iphelper->getExternalIP();

    if (QString::compare(ip, ipnew) == 0) {
        qDebug("ERROR: IPs are identical. Could not access TOR network on local port %i.", QString(crypt->decrypt(port)).toInt());
        qDebug("ERROR: Please make sure tor and torsocks is installed and configured.");
        qDebug("ERROR: The tor proxy must listen on the port configured in ghost.cfg.");
        qDebug("ERROR: If no port is set in the settings file, the default port 9050 will be used.");
        return false;
    } else {
        if (!silent) {
            qDebug("INFO: TOR is working as expected, your IP is masqueraded.");
        } //endif
    } //endif

    return true;
} //endfunction startTor

void GhostC::process() {
    validateArguments();

    if (validarg && QString::compare(args->at(1), "--help") == 0) {
        printHelp();
    } else if (validarg && QString::compare(args->at(1), "--help") != 0) {
        if (IOHelper::settingsReadable()) {
             settings = IOHelper::readEncryptedSettings();
             if (settings.contains("tor_port")) {
                 port = settings.value("tor_port");
             } //endif

             if (!settings.contains("db_port")) {
                 settings.insert("db_port", crypt->encrypt(QString::number(3306)));
             } //endif

             processArgs();
        } else {
            qDebug("ERROR: Config file /etc/ghost.cfg not readable.");
        } //endif
    } else {
        qDebug("ERROR: You provided invalid arguments.");
        printHelp();
    } //endif
} //endfunction process

void GhostC::validateArguments() {
    Validator *val = new Validator(this);

    if (QString::compare("--install", args->at(1)) == 0 && args->count() == 2) {
        validarg = true;
    } else if (QString::compare("--recrypt", args->at(1)) == 0 && args->count() == 2) {
        validarg = true;
    } else if (QString::compare("--auth", args->at(1)) == 0 && args->count() == 6 &&
               QString::compare(args->at(2), "--user") == 0 && val->isValidUserString(args->at(3)) &&
               QString::compare(args->at(4), "--pass") == 0 && val->isValidMD5Hash(args->at(5))) {
        validarg = true;
    } else if (QString::compare("--list-apikey", args->at(1)) == 0 && args->count() == 2) {
        validarg = true;
    } else if (QString::compare("--list-users", args->at(1)) == 0 && args->count() == 2) {
        validarg = true;
    } else if (QString::compare("--list-accounts", args->at(1)) == 0 && args->count() == 2) {
        validarg = true;
    } else if (QString::compare("--print-reqpwd", args->at(1)) == 0 && args->count() == 4 &&
               QString::compare("--id", args->at(2)) == 0 && val->isStdDbInt(args->at(3))) {
        validarg = true;
    } else if (QString::compare("--list-openreqs", args->at(1)) == 0 && args->count() == 4 &&
               QString::compare("--user", args->at(2)) == 0 && val->isValidUserString(args->at(3))) {
        validarg = true;
    } else if (QString::compare("--delreq", args->at(1)) == 0 && args->count() == 4 &&
               QString::compare("--id", args->at(2)) == 0 && val->isInt(args->at(3))) {
        validarg = true;
    } else if (QString::compare("--mail", args->at(1)) == 0 && args->count() == 2) {
        validarg = true;
    } else if (QString::compare("--initiate", args->at(1)) == 0 && args->count() == 6 &&
               QString::compare(args->at(2), "--user") == 0 && val->isValidUserString(args->at(3)) &&
               QString::compare(args->at(4), "--recp") == 0 && val->isValidUserString(args->at(5))) {
        validarg = true;
    } else if (QString::compare("--adduser", args->at(1)) == 0 && args->count() == 8 &&
            QString::compare(args->at(2), "--mail") == 0 && val->isValidMailAddress(args->at(3)) &&
            QString::compare(args->at(4), "--user") == 0 && val->isStdDbString(args->at(5)) &&
            QString::compare(args->at(6), "--pass") == 0 && val->isStdDbString(args->at(7))) {
        validarg = true;
    } else if (QString::compare("--deluser", args->at(1)) == 0 && args->count() == 3 &&
            val->isStdDbInt(args->at(2))) {
        validarg = true;
    } else if (QString::compare("--updateuser", args->at(1)) == 0 && args->count() == 8 &&
            QString::compare(args->at(2), "--id") == 0 && val->isStdDbInt(args->at(3)) &&
            QString::compare(args->at(4), "--attribute") == 0 && val->isValidUserAttribute(args->at(5), args->at(7)) &&
            QString::compare(args->at(6), "--value") == 0) {
        validarg = true;
    } else if (QString::compare("--changesettings", args->at(1)) == 0 && args->count() == 8 &&
            QString::compare(args->at(2), "--user") == 0 && val->isValidUserString(args->at(3)) &&
            QString::compare(args->at(4), "--mail") == 0 && (val->isValidMailAddress(args->at(5)) || QString::compare(args->at(5), "empty") == 0) &&
            QString::compare(args->at(6), "--pass") == 0 && (val->isValidMD5Hash(args->at(7)) || QString::compare(args->at(7), "empty") == 0)) {
        validarg = true;
    } else if (QString::compare("--addaccount", args->at(1)) == 0 &&
            args->count() > 15 && args->count() < 21 &&
            QString::compare(args->at(2), "--host") == 0 && val->isStdDbString(args->at(3)) &&
            QString::compare(args->at(4), "--port") == 0 && val->isValidPort(args->at(5)) &&
            QString::compare(args->at(6), "--auth") == 0 && val->isValidAuthValue(args->at(7)) &&
            QString::compare(args->at(8), "--encryption") == 0 && val->isValidEncryptionValue(args->at(9)) &&
            QString::compare(args->at(10), "--user") == 0 && val->isStdDbString(args->at(11)) &&
            QString::compare(args->at(12), "--pass") == 0 && val->isStdDbString(args->at(13)) &&
            QString::compare(args->at(14), "--mail") == 0 && val->isValidMailAddress(args->at(15)) &&
            QString::compare(args->at(16), "--max_recps") == 0 && val->isStdDbInt(args->at(17)) &&
            QString::compare(args->at(18), "--delay") == 0 && val->isStdDbInt(args->at(19))) {
        validarg = true;
    } else if (QString::compare("--delaccount", args->at(1)) == 0 && args->count() == 3 &&
        val->isStdDbInt(args->at(2))) {
        validarg = true;
    } else if (QString::compare("--updateaccount", args->at(1)) == 0 && args->count() == 8 &&
            QString::compare(args->at(2), "--id") == 0 && val->isStdDbInt(args->at(3)) &&
            QString::compare(args->at(4), "--attribute") == 0 && val->isValidAccountAttribute(args->at(5), args->at(7)) &&
            QString::compare(args->at(6), "--value") == 0) {
        validarg = true;
    } else if (QString::compare("--enterchat", args->at(1)) == 0 && args->count() == 6 &&
               QString::compare(args->at(2), "--user") == 0 && val->isValidUserString(args->at(3)) &&
               QString::compare(args->at(4), "--reqid") == 0 && val->isStdDbInt(args->at(5))) {
        validarg = true;
    } else if (QString::compare("--addchatmsg", args->at(1)) == 0 && args->count() == 8 &&
               QString::compare(args->at(2), "--reqid") == 0 && val->isStdDbInt(args->at(3)) &&
               QString::compare(args->at(4), "--user") == 0 && val->isValidUserString(args->at(5)) &&
               QString::compare(args->at(6), "--message") == 0 && args->at(6).length() > 5) {
        validarg = true;
    } else if (QString::compare("--getchatmessages", args->at(1)) == 0 && args->count() == 4 &&
               QString::compare(args->at(2), "--reqid") == 0 && val->isStdDbInt(args->at(3))) {
        validarg = true;
    } else if (QString::compare("--cleantimeouts", args->at(1)) == 0 && args->count() == 2) {
        validarg = true;
    } //endif

    delete val;
} //endfunction validateArguments

void GhostC::processArgs() {
    if (validarg && QString::compare("--install", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->doInstall();
    } else if (validarg && QString::compare("--recrypt", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->reencrypt();
    } else if (validarg && QString::compare("--auth", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->cleanTimeouts();
        gadmin->checkLogin(args->at(3), args->at(5));
    } else if (validarg && QString::compare("--list-apikey", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->printAPIKey();
    } else if (validarg && QString::compare("--list-users", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->printUserInfo();
    } else if (validarg && QString::compare("--list-accounts", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->printAccountInfo();
    } else if (validarg && QString::compare("--print-reqpwd", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->printRequestPassword(args->at(3));
    } else if (validarg && QString::compare("--list-openreqs", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->printOpenRequests(args->at(3));
    } else if (validarg && QString::compare("--delreq", args->at(1)) == 0) {
        DbHelper *dbh = new DbHelper(&settings, this);
        bool connected = dbh->connectToDb();
        bool admsuccess = false;
        bool mailsuccess = false;

        if (connected) {
            QList<QString> req = dbh->getUserRequest(args->at(3).toInt());
            if (req.count() < 8) {
                qDebug("ERROR: Could not get request from database.");
                exit(0);
            } //endif

            QList<QString> user1 = dbh->getDecryptedUser(req.at(1).toInt());
            QList<QString> user2 = dbh->getDecryptedUser(req.at(2).toInt());

            dbh->closeDbConnection();

            if (user1.count() < 4 || user2.count() < 4) {
                qDebug("ERROR: One of the users does not exist.");
            } else {
                QString subject = "Chat request deleted";
                QString text = "The chat request with the ID #" + args->at(3) + " has been deleted.";
                QStringList recipients;
                recipients << user1.at(2) << user2.at(2);

                GAdmin *gadmin = new GAdmin(&settings, this);
                admsuccess = gadmin->deleteChat(args->at(3));

                if (!admsuccess) {
                    qDebug("ERROR: Deleting the request failed.");
                    exit(0);
                } //endif

                bool tor = startTor(true);
                if (tor) {
                    GMailer *gm = new GMailer(&settings, this);
                    mailsuccess = gm->notify(recipients, subject, text, true);
                    if (!mailsuccess) {
                        qDebug("ERROR: Notification mail could not be sent.");
                    } //endif
                } //endif
            } //endif
        } else {
            qDebug("ERROR: Could not connect to database.");
        } //endif

        if (admsuccess && mailsuccess) {
            qDebug("1");
        } //endif

        delete dbh;
    } else if (validarg && QString::compare("--mail", args->at(1)) == 0) {
        bool tor = startTor();
        if (tor) {
            GMailer *gm = new GMailer(&settings, this);
            bool success = gm->sendEmails();
            if (success) {
                qDebug("INFO: Emails successfully sent.");
            } else {
                qDebug("WARNING: Some mails may not have been sent.");
            } //endif
        } //endif
    } else if (validarg && QString::compare("--initiate", args->at(1)) == 0) {
        QString subject = "New chat request";
        QString timeoutstr = crypt->decrypt(settings.value("request_timeout"));
        int timeout = timeoutstr.toInt();
        int mins = timeout / 60;
        QString text = "A new chat request has been initiated. Please log in to accept or delete it.";
        text.append("You have " + QString::number(mins) + " minutes left before it gets deleted automatically.\r\n\r\n");
        QString user = args->at(3);
        QString recp = args->at(5);

        GAdmin *gad = new GAdmin(&settings, this);
        if (gad->userExists(recp)) {
            bool tor = startTor();
            if (tor) {
                QString pass = crypt->generateKey();
                pass = pass.left(16);
                QString reqid = gad->writeNewRequest(user, recp, pass);

                text.append("The request ID is #" + reqid + "\r\n");
                text.append("The request password is: " + pass);

                GMailer *gm = new GMailer(&settings, this);
                gm->notify(QStringList() << user << recp, subject, text, true);

                qDebug("INFO: Success.");
            } //endif
        } else {
            qDebug("0");
        } //endif
    } else if (validarg && QString::compare("--adduser", args->at(1)) == 0) {
        QString mail = args->at(3);
        QString user = args->at(5);
        QString pass = args->at(7);

        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->addUser(mail, user, pass);

        QString url = gadmin->getCurrentAPIURL();
        QString subject = "Account created";
        QString text = "An account in the GHOST network has been created for you.\r\n";
        text.append("GHOST stands for 'Great Hope Of Secure Transmission' ");
        text.append("and you have been granted the great privilege to use it.\r\n\r\n");
        text.append("You will recieve your login credentials in person, not by mail.\r\n\r\n");
        text.append("NOTE: You should delete this mail for security reasons, it is no longer important.");

        QString urltext = "You recieve your first access URL with this mail. This URL is required to access the chat system.\r\n";
        urltext.append("Each Sunday at 0:00 PM you will recieve a new access URL by mail which is then again valid for one week.\r\n");
        urltext.append("You may only use the current access url, old access urls will not work.\r\n\r\n");
        urltext.append("The current URL is: " + url);

        bool tor = startTor();
        if (tor) {
            GMailer *gm = new GMailer(&settings, this);
            bool success1 = gm->notify(QStringList() << user, subject, text, true);
            if (success1) {
                qDebug("INFO: Successfully notified user about account creation.");
            } else {
                qDebug("ERROR: Account creation notification not sent. Either the recipient is invalid or there was a mail transport error.");
            } //endif

            bool success2 = gm->notify(QStringList() << user, QString("Current Access URL"), urltext, true);
            if (success2) {
                qDebug("INFO: Successfully mailed current API key to new user.");
            } else {
                qDebug("ERROR: Could not send API key. Either the recipient is invalid or there was a mail transport error.");
            } //endif
        } else {
            qDebug("ERROR: Could not initiate TOR connection. User has not been notified about account creation.");
        } //endif
    } else if (validarg && QString::compare("--deluser", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->delUser(args->at(2).toInt());
    } else if (validarg && QString::compare("--updateuser", args->at(1)) == 0) {
        int id = args->at(3).toInt();
        QString attribute = args->at(5);
        QString value = args->at(7);

        if (QString::compare(attribute, "password") == 0) {
            CryptoHelper *crh = new CryptoHelper("aes", 256, this);
            value = crypt->getPasswordHash(value);
        } //endif

        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->updateUser(id, attribute, value);
    } else if (validarg && QString::compare("--changesettings", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->changeSettings(args->at(3), args->at(5), args->at(7));
    } else if (validarg && QString::compare("--addaccount", args->at(1)) == 0) {
        QString host = args->at(3);
        QString portv = args->at(5);
        QString auth = args->at(7);
        QString ssl_tls = args->at(9);
        QString user = args->at(11);
        QString pass = args->at(13);
        QString email = args->at(15);
        QString max_recipients = (args->count() >= 18) ? args->at(17) : "";
        QString delay = (args->count() == 20) ? args->at(19) : "";

        GAdmin *admin = new GAdmin(&settings, this);
        admin->addMailAccount(host, portv, auth, ssl_tls, user, pass, email, max_recipients, delay);
    } else if (validarg && QString::compare("--delaccount", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->delMailAccount(args->at(2).toInt());
    } else if (validarg && QString::compare("--updateaccount", args->at(1)) == 0) {
        int id = args->at(3).toInt();
        QString attribute = args->at(5);
        QString value = args->at(7);
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->updateAccount(id, attribute, value);
    } else if (validarg && QString::compare("--enterchat", args->at(1)) == 0) {
        DbHelper *dbhelper = new DbHelper(&settings, this);
        bool connected = dbhelper->connectToDb();

        if (connected) {
            bool spamprotect = false;
            QList<QString> reqparams = dbhelper->getRequestProtectParams(args->at(3), args->at(5).toInt());
            int count = reqparams.at(1).toInt();
            int ctime = time(0);
            int ltime = reqparams.at(0).toInt();
            int tdiff = ctime - ltime;

            if ((count >=5 && tdiff < 7200 && ltime != 0) ||
                    (count < 5 && tdiff < 900 && ltime != 0)) {
                spamprotect = true;
            } //endif

            QList<QList<QString> > reqs = dbhelper->getUserRequests(args->at(3));

            dbhelper->closeDbConnection();
            delete dbhelper;

            QList<QString> request;
            for (int i=0; i<reqs.count(); i++) {
                if (QString::compare(reqs.at(i).at(0), args->at(5)) == 0) {
                    request = reqs.at(i);
                } //endif
            } //endfor

            GAdmin *gadmin = new GAdmin(&settings, this);
            gadmin->enterChat(args->at(3), args->at(5), reqparams.at(2));

            if (!spamprotect) {
                QString timeoutstr = crypt->decrypt(settings.value("request_timeout"));
                int timeout = timeoutstr.toInt();
                int mins = timeout / 60;
                QString subject = "Partner entered chat";
                QString text = "Your partner has entered the chat. The request id is " + args->at(5) + ". ";
                text.append("You have " + QString::number(mins) + " more minutes left before the request gets deleted automatically.");
                GMailer *gm = new GMailer(&settings, this);
                gm->notify(QStringList() << request[2], subject, text, true);
            } //endif
        } else {
            qDebug("ERROR: Could not connect to database.");
        } //endif
    } else if (validarg && QString::compare("--addchatmsg", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->addChatMessage(args->at(3).toInt(), args->at(5), args->at(7), 0, false);
    } else if (validarg && QString::compare("--getchatmessages", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->printChatMessages(args->at(3).toInt());
    } else if (validarg && QString::compare("--cleantimeouts", args->at(1)) == 0) {
        GAdmin *gadmin = new GAdmin(&settings, this);
        gadmin->cleanTimeouts();
    } //endif
} //endfunction processArgs
