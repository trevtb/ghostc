#ifndef GHOSTC_H
#define GHOSTC_H

#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QNetworkProxy>

#include <ipapihelper.h>
#include <gadmin.h>
#include <gmailer.h>
#include <validator.h>
#include <iohelper.h>
#include <cryptohelper.h>

class GhostC : public QObject {
    Q_OBJECT

private:
    QStringList *args;
    QString port;
    QMap<QString, QString> settings;
    bool validarg;
    CryptoHelper *crypt;

public:
    explicit GhostC(QObject *parent = 0);
    GhostC(QStringList *args, QObject *parent = 0);

signals:

private slots:
    void validateArguments();
    void processArgs();

public slots:
    void printHelp();
    bool startTor(bool);
    void process();
};

#endif // GHOSTC_H
