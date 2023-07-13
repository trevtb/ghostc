#include "cryptohelper.h"

CryptoHelper::CryptoHelper(QObject *parent) : QObject(parent) {
} //endconstructor

CryptoHelper::CryptoHelper(QString algorithm, int bitsize, QObject *parent) {
    this->algorithm = algorithm.toLower();
    this->bitsize = bitsize;

    if (!QCA::isSupported(QString(this->algorithm + QString::number(this->bitsize) + "-cbc").toStdString().c_str()) ||
            !QCA::isSupported("sha1")) {
        qDebug("ERROR: Encryption plugin not found - please check if libqca2-plugin-ossl is installed.");
        exit(0);
    } //endif
} //endconstructor

QString CryptoHelper::getNewAPIKey() {
    QCA::SecureArray secar(16);
    secar = QCA::Random::randomArray(16);

    return QCA::Hash("sha1").hashToString(secar);
} //endfunction generateAPIKey

void CryptoHelper::setKey(QCA::SecureArray key) {
    this->key = key;
} //endfunction setKey

QString CryptoHelper::encrypt(QString val) {
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    QCA::SecureArray secureData = val.toAscii();
    QCA::SecureArray ivval = generateIV();

    QCA::SymmetricKey symkey = QCA::SymmetricKey(key);
    QCA::InitializationVector iv = QCA::InitializationVector(ivval);

    QString algo = algorithm + QString::number(bitsize);
    QCA::Cipher cipher = QCA::Cipher(
                                        algo,
                                        QCA::Cipher::CBC,
                                        QCA::Cipher::DefaultPadding,
                                        QCA::Encode,
                                        symkey, iv
                                    );

    QCA::SecureArray encryptedData = cipher.process(secureData);

    if (!cipher.ok()) {
        qDebug() << "ERROR: Encryption failed.";
        exit(0);
    } //endif

    QString enchex = QString(QCA::arrayToHex(encryptedData.toByteArray()));
    QString ivhex = QString(QCA::arrayToHex(ivval.toByteArray()));
    QString encpackage = ivhex + enchex;

    return encpackage;
} //endfunction encrypt

QString CryptoHelper::decrypt(QString val) {
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    QStringList encpack = separateEncPackage(val);

    QCA::SecureArray encdata = QCA::hexToArray(encpack.at(1));

    QCA::SymmetricKey symkey = QCA::SymmetricKey(key);
    QCA::InitializationVector iv = QCA::InitializationVector(QCA::hexToArray(encpack.at(0)));

    QString algo = algorithm + QString::number(bitsize);
    QCA::Cipher cipher = QCA::Cipher(
                                        algo,
                                        QCA::Cipher::CBC,
                                        QCA::Cipher::DefaultPadding,
                                        QCA::Decode,
                                        symkey, iv
                                    );

    QCA::SecureArray decryptedData = cipher.process(encdata);

    if (!cipher.ok()) {
        qDebug() << "ERROR: Decryption failed! You might have data in your database encrypted with an old key.";
        exit(0);
    } //endif

    QString retVal(decryptedData.data());

    return retVal;
} //endfunction decrypt

QCA::SecureArray CryptoHelper::generateIV() {
    int s = bitsize / 8;
    QCA::SecureArray retVal(s);
    retVal = QCA::Random::randomArray(s);

    return retVal;
} //endfunction generateIV

QString CryptoHelper::generateKey() {
    int s = bitsize / 8;
    QCA::SecureArray secar(s);
    secar = QCA::Random::randomArray(s);
    QString retVal(QCA::arrayToHex(secar.toByteArray()));

    return retVal;
} //endfunction generateKey

QCA::SecureArray CryptoHelper::keyStringToSecureArray(QString keyval) {
    QByteArray keyar;
    keyar.append(keyval);

    QCA::SecureArray retVal(bitsize / 8);
    retVal = QCA::SecureArray(keyar);

    return retVal;
} //endfunction keyStringToSecureArray

QStringList CryptoHelper::separateEncPackage(QString package) {
    int s = (bitsize / 8) * 2;
    QString iv = package.left(s);
    QString enc = package.mid(s);

    QStringList retVal;
    retVal << iv << enc;

    return retVal;
} //endfunction separateEncPackage

QString CryptoHelper::getPasswordHash(QString password) {
    QByteArray val;
    val.append(password);

    QCA::Hash shaHash("sha1");
    shaHash.update(val);

    QByteArray hashResult = shaHash.final().toByteArray();
    QString midstr = QCA::arrayToHex(hashResult);
    midstr = midstr.mid(6,20);

    QByteArray val2;
    val2.append(midstr);
    QCA::Hash md5Hash("md5");
    md5Hash.update(val2);

    QString retVal = md5Hash.final().toByteArray().toHex();

    return retVal;
} //endfunction getPasswordHash
