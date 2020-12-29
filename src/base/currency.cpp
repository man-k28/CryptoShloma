#include "currency.h"

Currency::Currency(QObject *parent)
    : QObject(parent)
    , m_config(Config())
{}

Currency::Currency(const Currency::Config &config)
{
    setId(config.id);
    setConfig(config);
}

Currency::Currency(const Currency &o)
    : QObject(o.parent())
    , m_config(o.m_config)
{}

Currency::Currency(Currency &&o) noexcept
    : QObject(o.parent())
    , m_config(std::move(o.m_config))
{}

const QString &Currency::getName() const noexcept
{
    return m_config.name;
}

void Currency::setName(const QString &value) noexcept
{
    if ( m_config.name != value) {
        m_config.name = value;
        emit nameChanged();
    }
}

const QString &Currency::getSymbol() const noexcept
{
    return m_config.id.get();
}

const QString &Currency::getStatusMessage() const noexcept
{
    return m_config.statusMessage;
}

void Currency::setStatusMessage(const QString &value) noexcept
{
    if ( m_config.statusMessage != value ) {
        m_config.statusMessage = value;
        emit statusChanged();
    }
}

const Currency::CurrencyId &Currency::getId() const noexcept
{
    return m_config.id;
}

void Currency::setId(const CurrencyId &value) noexcept
{
    if ( m_config.id != value ) {
        m_config.id = value;
        emit idChanged();
    }
}

Currency::CurrencyStatusListing Currency::getListingStatus() const noexcept
{
    return m_config.listingStatus;
}

void Currency::setListingStatus(const CurrencyStatusListing value) noexcept
{
    if ( m_config.listingStatus != value ) {
        m_config.listingStatus = value;
        emit listingStatusChanged();
    }
}

Currency::CurrencyStatus Currency::getStatus() const noexcept
{
    return m_config.status;
}

void Currency::setStatus(const CurrencyStatus value) noexcept
{
    if ( m_config.status != value ) {
        m_config.status = value;
        emit statusChanged();
    }
}

void Currency::setConfig(const Config &config) noexcept
{
    setName(config.name);
    setStatusMessage(config.statusMessage);
    setStatus(config.status);
    setListingStatus(config.listingStatus);
}
