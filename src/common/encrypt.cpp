#include "encrypt.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <common/constants.h>

QByteArray Encrypt::sha256(const QByteArray& text)
{
    unsigned int outLen = 0;
    QByteArray dataBuff;
    dataBuff.resize(EVP_MAX_MD_SIZE);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_MD_CTX* evpMdCtx = EVP_MD_CTX_create();
#else
    EVP_MD_CTX* evpMdCtx = EVP_MD_CTX_new();
#endif
    EVP_DigestInit(evpMdCtx, EVP_sha256());
    EVP_DigestUpdate(evpMdCtx, text.data(), static_cast<size_t>(text.size()));
    EVP_DigestFinal_ex(evpMdCtx, reinterpret_cast<unsigned char*>(dataBuff.data()), &outLen);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_MD_CTX_cleanup(evpMdCtx);
    OPENSSL_free(evpMdCtx);
#else
    EVP_MD_CTX_free(evpMdCtx);
#endif
    dataBuff.resize(static_cast<int>(outLen));
    return dataBuff.toHex();
}

QByteArray Encrypt::encrypt(const QByteArray& data, const QByteArray& password)
{
    int outLen = 0;
    QByteArray dataBuff;
    dataBuff.resize(data.size() + AES_BLOCK_SIZE);
    EVP_CIPHER_CTX* evpCipherCtx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(evpCipherCtx);
    EVP_EncryptInit(evpCipherCtx, EVP_aes_256_cbc(), reinterpret_cast<const unsigned char*>(sha256(password).data()),
                    reinterpret_cast<const unsigned char*>(sha256(GLOBAL_APP_NAME.toStdString().c_str() + password).data()));
    EVP_EncryptUpdate(evpCipherCtx, reinterpret_cast<unsigned char*>(dataBuff.data()), &outLen,
                      reinterpret_cast<const unsigned char*>(data.data()),
                      data.size());
    int tempLen = outLen;
    EVP_EncryptFinal(evpCipherCtx, reinterpret_cast<unsigned char*>(dataBuff.data()) + tempLen, &outLen);
    tempLen += outLen;
    EVP_CIPHER_CTX_cleanup(evpCipherCtx);
    EVP_CIPHER_CTX_free(evpCipherCtx);
    dataBuff.resize(tempLen);
    return dataBuff;
}

QByteArray Encrypt::decrypt(const QByteArray& data, const QByteArray& password)
{
    int outLen = 0;
    QByteArray dataBuff;
    dataBuff.resize(data.size() + AES_BLOCK_SIZE);
    EVP_CIPHER_CTX* evpCipherCtx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(evpCipherCtx);
    EVP_DecryptInit(evpCipherCtx, EVP_aes_256_cbc(), reinterpret_cast<const unsigned char*>(sha256(password).data()),
                    reinterpret_cast<const unsigned char*>(sha256(GLOBAL_APP_NAME.toStdString().c_str() + password).data()));
    EVP_DecryptUpdate(evpCipherCtx, reinterpret_cast<unsigned char*>(dataBuff.data()), &outLen,
                      reinterpret_cast<const unsigned char*>(data.data()),
                      data.size());
    int tempLen = outLen;
    EVP_DecryptFinal(evpCipherCtx, reinterpret_cast<unsigned char*>(dataBuff.data()) + tempLen, &outLen);
    tempLen += outLen;
    EVP_CIPHER_CTX_cleanup(evpCipherCtx);
    EVP_CIPHER_CTX_free(evpCipherCtx);
    dataBuff.resize(tempLen);
    return dataBuff;
}
