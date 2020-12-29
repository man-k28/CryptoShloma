#include "order.h"

Order::Order(QObject *parent)
    : QObject(parent)
    , m_config(Config())
{}

Order::Order(const Order::Config &config)
{
    setOrderId(config.orderId);
    setConfig(config);
}

Order::Order(const Order &o) noexcept
    : QObject (o.parent())
    , m_config(o.m_config)
    , m_market(o.getMarket())
{}

Order::Order(Order &&o) noexcept
    : QObject (o.parent())
    , m_config(std::move(o.m_config))
    , m_market(std::move(o.m_market))
{}

QString Order::toOrderTypeString(Order::OrderType type) noexcept
{
    return type == Buy ? QString(QStringLiteral("Buy")) : QString(QStringLiteral("Sell"));
}

Market *Order::getMarketPtr() const noexcept
{
    return m_market.constCast<Market>().get();
}

const Market::ConstPtr &Order::getMarket() const noexcept
{
    return m_market;
}

void Order::setMarket(const Market::ConstPtr &value) noexcept
{
    m_market = value;
}

const QDateTime &Order::getTimeStamp() const noexcept
{
    return m_config.timeStamp;
}

void Order::setTimeStamp(const QDateTime &value) noexcept
{
    if ( m_config.timeStamp != value
         && !value.isNull() ) {
        m_config.timeStamp = value;
        emit timeStampChanged();
    }
}

const QDateTime &Order::getUpdatedTimeStamp() const noexcept
{
    return m_config.updatedTimeStamp;
}

void Order::setUpdatedTimeStamp(const QDateTime &value) noexcept
{
    if ( m_config.updatedTimeStamp != value
         && !value.isNull() ) {
        m_config.updatedTimeStamp = value;
        emit updatedTimeStampChanged();
    }
}

const qreal &Order::getPrice() const noexcept
{
    return m_config.price;
}

void Order::setPrice(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.price, value)
         && !qFuzzyIsNull(value) ) {
        m_config.price = value;
        emit priceChanged();
    }
}

const qreal &Order::getCurrencyVolume() const noexcept
{
    return m_config.currencyVolume;
}

void Order::setCurrencyVolume(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.currencyVolume, value)
         && !qFuzzyIsNull(value) ) {
        m_config.currencyVolume = value;
        emit currencyVolumeChanged();
    }
}

//const qreal &Order::getVolume() const noexcept
//{
//    return m_config.volume;
//}

//void Order::setVolume(const qreal &value) noexcept
//{
//    if ( !qFuzzyCompare(m_config.volume, value) ) {
//        m_config.volume = value;
//        emit volumeChanged();
//    }
//}

const qreal &Order::getBaseVolume() const noexcept
{
    return m_config.baseVolume;
}

void Order::setBaseVolume(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.baseVolume, value)
         && !qFuzzyIsNull(value) ) {
        m_config.baseVolume = value;
        emit baseVolumeChanged();
    }
}

const qreal &Order::getFee() const noexcept
{
    return m_config.fee;
}

void Order::setFee(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.fee, value)
         && !qFuzzyIsNull(value) ) {
        m_config.fee = value;
        emit feeChanged();
    }
}

const qreal &Order::getRemaining() const noexcept
{
    return m_config.remaining;
}

void Order::setRemaining(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.remaining, value) ) {
        m_config.remaining = value;
        emit remainingChanged();
    }
}

const Order::OrderId &Order::getOrderId() const noexcept
{
    return m_config.orderId;
}

void Order::setOrderId(const OrderId &value) noexcept
{
    if ( m_config.orderId != value ) {
        m_config.orderId = value;
        emit orderIdChanged();
    }
}

Order::OrderType Order::getType() const noexcept
{
    return m_config.type;
}

void Order::setType(const OrderType value) noexcept
{
    if (m_config.type != value ) {
        m_config.type = value;
        emit typeChanged();
    }
}

void Order::setConfig(const Order::Config &config) noexcept
{
    setTimeStamp(config.timeStamp);
    setUpdatedTimeStamp(config.updatedTimeStamp);
    setPrice(config.price);
    setCurrencyVolume(config.currencyVolume);
    setBaseVolume(config.baseVolume);
    setFee(config.fee);
    setRemaining(config.remaining);
    setType(config.type);
}
