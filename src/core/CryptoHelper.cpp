#include "CryptoHelper.h"
#include <QCryptographicHash>
#include <QByteArray>

QByteArray CryptoHelper::getKey() {
    // 实际应用中应该使用更安全的密钥管理
    return QCryptographicHash::hash("TerminalManagerSecretKey", QCryptographicHash::Sha256);
}

QString CryptoHelper::encrypt(const QString& plainText) {
    if (plainText.isEmpty()) return {};

    QByteArray key = getKey();
    QByteArray data = plainText.toUtf8();
    QByteArray encrypted;

    for (int i = 0; i < data.size(); ++i) {
        encrypted.append(static_cast<char>(data[i] ^ key[i % key.size()]));
    }

    return encrypted.toBase64();
}

QString CryptoHelper::decrypt(const QString& cipherText) {
    if (cipherText.isEmpty()) return {};

    QByteArray key = getKey();
    QByteArray data = QByteArray::fromBase64(cipherText.toUtf8());
    QByteArray decrypted;

    for (int i = 0; i < data.size(); ++i) {
        decrypted.append(static_cast<char>(data[i] ^ key[i % key.size()]));
    }

    return QString::fromUtf8(decrypted);
}
