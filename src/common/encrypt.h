#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <QByteArray>

class Encrypt
{
public:
    Encrypt() = default;
    static QByteArray sha256(const QByteArray& text);
    static QByteArray decrypt(const QByteArray& data, const QByteArray& password);
    static QByteArray encrypt(const QByteArray& data, const QByteArray& password);
};

#endif // ENCRYPT_H
