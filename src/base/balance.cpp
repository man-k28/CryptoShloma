#include "balance.h"

Balance::Balance(QObject *parent)
    : QObject(parent)
    , m_config(Config())
{}

Balance::Balance(const Balance::Config &config)
{
    setConfig(config);
}

Balance::Balance(const Balance &o) noexcept
    : QObject (o.parent())
    , m_currency(o.getCurrency())
    , m_config(o.m_config)
{}

Balance::Balance(Balance &&o) noexcept
    : QObject (o.parent())
    , m_currency(std::move(o.m_currency))
    , m_config(std::move(o.m_config))
{}

Currency *Balance::getCurrencyPtr() const noexcept
{
    return m_currency.constCast<Currency>().get();
}

const Currency::ConstPtr Balance::getCurrency() const noexcept
{
    return m_currency;
}

void Balance::setCurrency(const Currency::ConstPtr &value) noexcept
{
    m_currency = value;
}

const QString &Balance::getStatusMessage() const noexcept
{
    return m_config.statusMessage;
}

void Balance::setStatusMessage(const QString &value) noexcept
{
    if ( m_config.statusMessage != value ) {
        m_config.statusMessage = value;
        emit statusMessageChanged();
    }
}

const qreal &Balance::getTotal() const noexcept
{
    return m_config.total;
}

void Balance::setTotal(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.total, value) ) {
        m_config.total = value;
        emit totalChanged();
    }
}

const qreal &Balance::getAvailable() const noexcept
{
    return m_config.available;
}

void Balance::setAvailable(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.available,value) ) {
        m_config.available = value;
        emit availableChanged();
    }
}

const qreal &Balance::getHeldForTrades() const noexcept
{
    return m_config.heldForTrades;
}

void Balance::setHeldForTrades(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.heldForTrades, value) ) {
        m_config.heldForTrades = value;
        emit heldForTradesChanged();
    }
}

Balance::BalanceStatus Balance::getStatus() const noexcept
{
    return m_config.status;
}

void Balance::setStatus(const BalanceStatus value) noexcept
{
    if ( m_config.status != value ) {
        m_config.status = value;
        emit statusChanged();
    }
}

const QDateTime &Balance::getTimestamp() const noexcept
{
    return m_config.timestamp;
}

void Balance::setTimestamp(const QDateTime &value) noexcept
{
    if ( m_config.timestamp != value ) {
        m_config.timestamp = value;
    }
}

void Balance::setConfig(const Balance::Config &config) noexcept
{
    setTotal(config.total);
    setAvailable(config.available);
    setStatusMessage(config.statusMessage);
    setHeldForTrades(config.heldForTrades);
    setStatus(config.status);
    setTimestamp(config.timestamp);
}
