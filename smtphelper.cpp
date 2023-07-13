#include "smtphelper.h"

SmtpHelper::SmtpHelper(QObject *parent) : QObject(parent) {
} //endconstructor

SmtpHelper::SmtpHelper(QMap<QString, QString> *settings, QObject *parent) {
    this->settings = settings;
    initialize();
} //endconstructor

bool SmtpHelper::sendMails(QString url) {
    bool retVal = true;
    this->message = "Current access URL: " + url;
    this->subject = "Access URL";

    QStringList recipients = getRecipients();
    if (recipients.count() < 1) {
        qDebug("INFO: There are no users in the database, exiting.");
        return false;
    } //endif

    QList<QStringList> recar = getRecipientArray(recipients, recipients_per_mail);

    qDebug("INFO: Sending new API key to %i users.", recipients.count());
    if (recar.count() > 1) {
        qDebug("INFO: Recipients will be split on %i mails.", recar.count());
    } else {
        qDebug("INFO: Packaging recipients into a single mail.");
    } //endif

    SmtpClient smtpc;

    for (int i=0; i<recar.count(); i++) {
        bool sent = false;
        int counter = 0;

        while (!sent && counter < 3) {
            if (counter != 0) {
                initialize();
            } //endif

            qDebug("INFO: Using %s via %s for transport of mail #%i.", from.toStdString().c_str(), host.toStdString().c_str(), (i+1));

            smtpc.setHost(host);
            smtpc.setPort(port);

            if (ssl == 0) {
                smtpc.setConnectionType(SmtpClient::TcpConnection);
            } else if (ssl == 1) {
                smtpc.setConnectionType(SmtpClient::SslConnection);
            } else if (ssl == 2) {
                smtpc.setConnectionType(SmtpClient::TlsConnection);
            } //endif

            smtpc.setUser(user);
            smtpc.setPassword(pass);

            if (auth == 0) {
                smtpc.setAuthMethod(SmtpClient::AuthPlain);
            } else if (auth == 1) {
                smtpc.setAuthMethod(SmtpClient::AuthLogin);
            } //endif

            MimeText text;
            text.setText(message);

            MimeMessage msg;
            msg.setSender(new EmailAddress(from, "GHOST"));
            msg.addRecipient(new EmailAddress(recar[i][0], "GHOST User"));

            for (int j=1; j<recar[i].count(); j++) {
               msg.addBcc(new EmailAddress(recar[i][j], "GHOST User"));
            } //endfor

            msg.setSubject(subject);
            msg.addPart(&text);

            QString msgString = "";
            if (!smtpc.connectToHost()) {
                msgString.append("ERROR: Connection failed.");
            } //endif

            if (!smtpc.login()) {
                msgString.append("ERROR: Authentication failed.");
            } //endif

            if (!smtpc.sendMail(msg)) {
                msgString.append("ERROR: Server rejected message.");
            } //endif

            smtpc.quit();

            if (msgString.size() < 2) {
                sent = true;
            } else {
                qDebug("WARNING: Sending message failed, retrying with different account.");
            } //endif

            counter++;

            if (counter == 3 && msgString.size() > 1) {
                qDebug(msgString.toStdString().c_str());
                retVal = false;
            } //endif
        } //endwhile

        if ((i + 1) < recar.count()) {
            qDebug("INFO: Waiting 6 seconds before sending next message.");
            QMutex mutex;
            mutex.lock();
            QWaitCondition waitCondition;
            waitCondition.wait(&mutex, send_delay);
            mutex.unlock();
        } //endif

        initialize();
    } //endfor

    return retVal;
} //endfunction sendMails

bool SmtpHelper::sendNotify(QStringList emails, QString subject, QString messag, bool silent) {
    bool retVal = true;
    this->subject = subject;
    this->message = messag;

    QString mailsstring;
    for (int i=0; i<emails.count(); i++) {
        mailsstring.append(emails.at(i));
        if (i < (emails.count() - 1)) {
            mailsstring.append(",");
        } //endif
    } //endfor

    if (!silent) {
        qDebug("INFO: Sending notification to %s.", mailsstring.toStdString().c_str());
    } //endif

    bool sent = false;
    int counter = 0;
    while (!sent && counter < 3) {
        if (counter != 0) {
            initialize();
        } //endif

        SmtpClient smtpc;
        smtpc.setHost(this->host);
        smtpc.setPort(this->port);

        if (this->ssl == 0) {
            smtpc.setConnectionType(SmtpClient::TcpConnection);
        } else if (this->ssl == 1) {
            smtpc.setConnectionType(SmtpClient::SslConnection);
        } else if (this->ssl == 2) {
            smtpc.setConnectionType(SmtpClient::TlsConnection);
        } //endif

        smtpc.setUser(this->user);
        smtpc.setPassword(this->pass);

        if (this->auth == 0) {
            smtpc.setAuthMethod(SmtpClient::AuthPlain);
        } else if (this->auth == 1) {
            smtpc.setAuthMethod(SmtpClient::AuthLogin);
        } //endif

        MimeText text;
        text.setText(this->message);

        MimeMessage msg;
        msg.setSender(new EmailAddress(this->from, "GHOST"));
        msg.addRecipient(new EmailAddress(emails.at(0), "GHOST User"));

        if (emails.count() > 1) {
            for (int i=1; i<emails.count(); i++) {
                msg.addBcc(new EmailAddress(emails.at(i), "GHOST User"));
            } //endfor
        } //endif

        msg.setSubject(this->subject);
        msg.addPart(&text);

        QString msgString = "";
        if (!smtpc.connectToHost()) {
            msgString.append("ERROR: Connection failed.");
        } //endif

        if (!smtpc.login()) {
            msgString.append("ERROR: Authentication failed.");
        } //endif

        if (!smtpc.sendMail(msg)) {
            msgString.append("ERROR: Server rejected message.");
        } //endif

        smtpc.quit();

        if (msgString.size() < 2) {
            sent = true;
        } else {
            if (!silent) {
                qDebug("WARNING: Sending message failed, retrying with different account.");
            } //endif
        } //endif

        counter++;

        if (counter == 3 && msgString.size() > 1) {
            qDebug(msgString.toStdString().c_str());
            retVal = false;
        } //endif
    } //endwhile

    return retVal;
} //endfunction sendNotify

QStringList SmtpHelper::getRecipients() {
    DbHelper *db_ref = new DbHelper(settings, this);
    db_ref->connectToDb();

    QList<QList<QString> > users = db_ref->getDecryptedTable("user");
    db_ref->closeDbConnection();
    delete db_ref;

    QStringList recipients;
    for (int i=0; i<users.count(); i++) {
        recipients << users[i][1];
    } //endfor

    return recipients;
} //endfunction getRecipients

QList<QStringList> SmtpHelper::getRecipientArray(QStringList recipients, int max_recipients) {
    QList<QStringList> recar;

    int arcount = recipients.count() / max_recipients;
    int multiplied = arcount * max_recipients;
    if (multiplied < recipients.count()) {
        arcount++;
    } //endif

    int count = 0;
    for (int i=0; i<arcount; i++) {
        QStringList row;
        for (int j=0; j<max_recipients; j++) {
            if (count < recipients.count()) {
                row << recipients[count];
            } //endif
             count++;
        } //endfor
        recar.append(row); 
    } //endfor

    return recar;
} //endfunction getRecipientArray

QList<QString> SmtpHelper::getMailAccount() {
    DbHelper *db_ref = new DbHelper(settings, this);
    db_ref->connectToDb();

    QList<QList<QString> > accounts = db_ref->getDecryptedTable("mailaccount");
    if (accounts.count() < 1) {
        qDebug("INFO: You have no mail accounts set up, exiting.");
        exit(0);
    } //endif

    QList<QString> account = accounts[0];
    if (accounts.size() > 1) {
        int numa = 0;
        int numb = 0;
        int first = INT_MAX;
        int second = INT_MAX;
        for (int i=0; i<accounts.size() ; i++) {
            if (accounts[i][10].toInt() < first) {
                second = first;
                first = accounts[i][10].toInt();
                numb = numa;
                numa = i;
            } else if (accounts[i][10].toInt() < second && accounts[i][10].toInt() != first) {
                second = accounts[i][10].toInt();
                numb = i;
            } //endif
        } //endfor

        account = accounts[numa];
    } //endif

    db_ref->incrementAccountCounter(account[0].toInt());

    db_ref->closeDbConnection();
    delete db_ref;

    return account;
} //endfunction getMailAccount

void SmtpHelper::initialize() {
    QList<QString> account = getMailAccount();

    host = account[1];

    port = account[2].toInt();
    auth = account[3].toInt();
    ssl = account[4].toInt();
    user = account[5];
    pass = account[6];
    from = account[7];
    recipients_per_mail = account[8].toInt();
    send_delay = account[9].toInt();
} //endfunction initialize
