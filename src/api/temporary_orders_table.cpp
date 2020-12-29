#include "temporary_orders_table.h"
#include <Logger.h>
#include <common/constants.h>

TemporaryOrdersTable::TemporaryOrdersTable(QObject *parent) noexcept
    : QObject(parent)
{
}

void TemporaryOrdersTable::updatePlaceOrder(const OrderRecord &record)
{
    auto found = std::find_if(m_data.begin(), m_data.end(), [&record](const OrderRecord &rec){
        return ( (rec.marketId == record.marketId) &&
                 qFuzzyCompare(rec.price, record.price) &&
                 qFuzzyCompare(rec.baseVolume, record.baseVolume) &&
                 qFuzzyCompare(rec.volume, record.volume)
               );
    });
    if ( found != m_data.end() ) {
        LOG_TRACE(QString(QStringLiteral("Temp %1 order for market %2 was placed. volume=%3 baseVolume=%4 price=%5 timestamp=%6"))
                 .arg(Order::toOrderTypeString(record.orderType))
                 .arg(record.marketId.get())
                 .arg(DOUBLE_TO_STR(record.volume))
                 .arg(DOUBLE_TO_STR(record.baseVolume))
                 .arg(DOUBLE_TO_STR(record.price))
                 .arg(record.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
                 );
        found->orderId = record.orderId;
        found->timestamp = record.timestamp;
    } else {
        LOG_TRACE(QString(QStringLiteral("Not found %1 temp order for market %2 for place updated. volume=%3 baseVolume=%4 price=%5 timestamp=%6"))
                    .arg(Order::toOrderTypeString(record.orderType))
                    .arg(record.marketId.get())
                    .arg(DOUBLE_TO_STR(record.volume))
                    .arg(DOUBLE_TO_STR(record.baseVolume))
                    .arg(DOUBLE_TO_STR(record.price))
                    .arg(record.timestamp.toString("yyyy-MM-dd hh:mm:ss")));
    }
}

void TemporaryOrdersTable::updateEraseOrder(const OrderRecord &record)
{
    QList<TemporaryOrdersTable::OrderRecord>::iterator found = m_data.end();
    if ( record.orderId.isValid() ) {
        found = std::find_if(m_data.begin(), m_data.end(), [&record](const OrderRecord &rec){
            return ( rec.orderId == record.orderId );
        });
    }

    if ( found == m_data.end() ) {
        found = std::find_if(m_data.begin(), m_data.end(), [&record](const OrderRecord &rec){
            return ( (rec.marketId == record.marketId) &&
                    qFuzzyCompare(rec.price, record.price) &&
                    qFuzzyCompare(rec.baseVolume, record.baseVolume) &&
                    qFuzzyCompare(rec.volume, record.volume)
                    );
        });
    }

    if ( found != m_data.end() ) {
        m_data.erase(found);
        LOG_TRACE(QString(QStringLiteral("Temp %1 order for market %2 was removed. volume=%3 baseVolume=%4 price=%5 timestamp=%6"))
                 .arg(Order::toOrderTypeString(record.orderType))
                 .arg(record.marketId.get())
                 .arg(DOUBLE_TO_STR(record.volume))
                 .arg(DOUBLE_TO_STR(record.baseVolume))
                 .arg(DOUBLE_TO_STR(record.price))
                 .arg(record.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
                 );
    } else {
        LOG_TRACE(QString(QStringLiteral("Not found %1 temp order for market %2 for removed. volume=%3 baseVolume=%4 price=%5 timestamp=%6"))
                    .arg(Order::toOrderTypeString(record.orderType))
                    .arg(record.marketId.get())
                    .arg(DOUBLE_TO_STR(record.volume))
                    .arg(DOUBLE_TO_STR(record.baseVolume))
                    .arg(DOUBLE_TO_STR(record.price))
                    .arg(record.timestamp.toString("yyyy-MM-dd hh:mm:ss")));
    }
}

void TemporaryOrdersTable::updateEraseOrder(const Order::OrderId &orderId)
{
    auto found = std::find_if(m_data.begin(), m_data.end(), [&orderId](const OrderRecord &rec){
        return ( rec.orderId == orderId );
    });
    if ( found != m_data.end() ) {
        LOG_TRACE(QString(QStringLiteral("Temp order for market %1 was removed")).arg(found->marketId.get()));
        m_data.erase(found);
    } else {
        LOG_TRACE(QString(QStringLiteral("Not found temp order id %1 for removed")).arg(orderId.get()));
    }
}

bool TemporaryOrdersTable::submitOrder(const OrderRecord &record)
{
    auto found = std::find_if(m_data.begin(), m_data.end(), [&record](const OrderRecord &rec){
        //NOTE: проверяем, если на паре есть уже такой тип ордера,
        // то не разрешаем ставить дублирующий или второй по типу ордер.
        return ( (record.orderType == rec.orderType) && (record.marketId == rec.marketId) );
    });
    if ( found == m_data.end() ) {
        m_data << record;
        LOG_TRACE(QString(QStringLiteral("Temp %1 order for market %2 was added. volume=%3 baseVolume=%4 price=%5 timestamp=%6"))
                 .arg(Order::toOrderTypeString(record.orderType))
                 .arg(record.marketId.get())
                 .arg(DOUBLE_TO_STR(record.volume))
                 .arg(DOUBLE_TO_STR(record.baseVolume))
                 .arg(DOUBLE_TO_STR(record.price))
                 .arg(record.timestamp.toString("yyyy-MM-dd hh:mm:ss")));
        return true;
    } else {
        LOG_TRACE(QString(QStringLiteral("Found of dublicate %1 order for %2. volume=%3 baseVolume=%4 price=%5 timestamp=%6. Exist: volume=%7 baseVolume=%8 price=%9 timestamp=%10."))
                    .arg(Order::toOrderTypeString(record.orderType))
                    .arg(record.marketId.get())
                    .arg(DOUBLE_TO_STR(record.volume))
                    .arg(DOUBLE_TO_STR(record.baseVolume))
                    .arg(DOUBLE_TO_STR(record.price))
                    .arg(record.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
                    .arg(DOUBLE_TO_STR(found->volume))
                    .arg(DOUBLE_TO_STR(found->baseVolume))
                    .arg(DOUBLE_TO_STR(found->price))
                    .arg(found->timestamp.toString("yyyy-MM-dd hh:mm:ss")));
        return false;
    }
}

TemporaryOrdersTable::OrderRecord::OrderRecord(const Market::MarketId &marketId,
                                               Order::OrderType type,
                                               const qreal &price,
                                               const qreal &baseVolume,
                                               const qreal &volume) noexcept
    : marketId(marketId)
    , orderType(type)
    , volume(volume)
    , baseVolume(baseVolume)
    , price(price)
    , timestamp(QDateTime::currentDateTime())
{
}

TemporaryOrdersTable::OrderRecord::OrderRecord(const Market::MarketId &marketId,
                                               const Order::OrderId &orderId,
                                               const QDateTime &timestamp,
                                               const qreal &price,
                                               const qreal &baseVolume,
                                               Order::OrderType type,
                                               const qreal &volume) noexcept
    : marketId(marketId)
    , orderId(orderId)
    , orderType(type)
    , volume(volume)
    , baseVolume(baseVolume)
    , price(price)
    , timestamp(timestamp)
{
}
