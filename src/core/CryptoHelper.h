#ifndef CRYPTOHELPER_H
#define CRYPTOHELPER_H

#include <QString>

class CryptoHelper {
public:
    static QString encrypt(const QString& plainText);
    static QString decrypt(const QString& cipherText);

private:
    static QByteArray getKey();
};

#endif // CRYPTOHELPER_H
