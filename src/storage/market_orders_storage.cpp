#include "market_orders_storage.h"
#include "markets_storage.h"
#include <api/i_exchange_api.h>
#include <Logger.h>
#include <QWriteLocker>

MarketOrdersStorage::MarketOrdersStorage(IExchangeAPI *parent) noexcept
    : QObject()
    , m_api(parent)
{
}

Order::ConstContainer MarketOrdersStorage::filter(const Market::MarketId &marketId, const Order::OrderType orderType) const noexcept
{
    Order::ConstContainer filtered;

    std::copy_if(m_Data.begin(), m_Data.end(),std::back_inserter(filtered), [&marketId, orderType](const Order::Ptr &order){
        return (order->getMarket()->getId() == marketId) && (order->getType() == orderType);
    });
    std::sort(filtered.begin(), filtered.end(),[](const Order::ConstPtr &leftOrder, const Order::ConstPtr &rightOrder){
        return leftOrder->getTimeStamp() > rightOrder->getTimeStamp();
    });
    return filtered;
}

void MarketOrdersStorage::insert(const Market::MarketId &id) noexcept
{
    QReadLocker locked(&lock);
    if ( !m_Filters.contains(id) )
        m_Filters << id;
}

void MarketOrdersStorage::merge(const QList<exchange::OrderEntry> &data) noexcept
{
    LOG_TRACE_TIME();
    QReadLocker locked(&lock);
    QList<Market::MarketId> ids;
    for (auto i = data.cbegin(); i != data.cend(); ++i) {
        const auto &id = (*i).marketId;
        if ( !ids.contains(id) )
            ids << id;
    }
    for (auto item = m_Data.begin(); item != m_Data.end();) {
        if ( ids.contains(item->get()->getMarket()->getId()))
            item = m_Data.erase(item);
        else
            ++item;
    }

    for (auto item : data ) {
        if ( m_Filters.contains(item.marketId) ) {
            auto market = m_api->marketsStorage()->findMarketById(item.marketId);
            if ( !market.isNull() ) {
                auto order = Order::Ptr::create(item.params);
                order->setMarket(market);
                m_Data.append(order);
            }
        }
    }
}

void MarketOrdersStorage::remove(const Market::MarketId &id) noexcept
{
    LOG_TRACE_TIME();
    QReadLocker locked(&lock);
    m_Filters.removeAll(id);
    for (auto item = m_Data.begin(); item != m_Data.end();) {
        if ( item->get()->getMarket()->getId() == id ) {
            item = m_Data.erase(item);
        } else
            ++item;
    }
}
