#ifndef CRYPTOSHLOMA_H
#define CRYPTOSHLOMA_H
#include <QQmlApplicationEngine>
#include "models/exchange_pool.h"
#include "include/version.hpp"

class CryptoShloma final: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString softwareVersion READ getSoftwareVersion CONSTANT)
public:
    explicit CryptoShloma(QObject *parent = nullptr);
    ~CryptoShloma() noexcept;
    bool init();
    const QString getSoftwareVersion() const { return cryptoShlomaVersionStr(); }
public slots:
    void reloadSettings(const bool initial = false) noexcept;
    void restartEmulation() noexcept;
private:
    void qmlRegistration() noexcept;

    QQmlApplicationEngine   m_Engine{};
    ExchangePool            m_exchangePool{};
};

#endif // CRYPTOSHLOMA_H
