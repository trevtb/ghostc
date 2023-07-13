#ifndef IOHELPER_H
#define IOHELPER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QFileInfo>
#include <QDebug>
#include <cryptohelper.h>

class IOHelper : public QObject {
    Q_OBJECT

private:
    static QString dirpath;
    static QString cfgname;
    static QString keyname;

public:
    explicit IOHelper(QObject *parent = 0);
    
signals:

private slots:
    static QMap<QString, QString> readSettings();

public slots:
    static QMap<QString, QString> readEncryptedSettings();
    static bool settingsReadable();
    static bool keyFileValid();
    static void writeNewKey(QString);
    static QCA::SecureArray readKey();

};

#endif // IOHELPER_H
