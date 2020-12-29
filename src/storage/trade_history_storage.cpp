#include "trade_history_storage.h"
#include <api/i_exchange_api.h>
#include <Logger.h>
#include "markets_storage.h"

TradeHistoryStorage::TradeHistoryStorage(IExchangeAPI *parent) noexcept
    : QObject()
    , m_api(parent)
{
}

Order::ConstContainer TradeHistoryStorage::filter(const Market::MarketId &marketId) const noexcept
{
    Order::ConstContainer filtered;

    std::copy_if(m_Data.begin(), m_Data.end(),std::back_inserter(filtered), [&marketId](const Order::Ptr &order){
        return order->getMarket()->getId() == marketId;
    });
    std::sort(filtered.begin(), filtered.end(),[](const Order::ConstPtr &leftOrder, const Order::ConstPtr &rightOrder){
        return leftOrder->getTimeStamp() > rightOrder->getTimeStamp();
    });
    return filtered;
}

void TradeHistoryStorage::insert(const Market::MarketId &id) noexcept
{
    QReadLocker locked(&lock);
    if ( !m_Filters.contains(id) )
        m_Filters << id;
}

void TradeHistoryStorage::merge(const QList<exchange::OrderEntry> &data) noexcept
{
    LOG_TRACE_TIME();
    QReadLocker locked(&lock);
    for (const auto &item : data) {
        auto found = std::find_if(m_Data.begin(), m_Data.end(),
            [ &item ]( const decltype( m_Data )::value_type &order ) noexcept
            {
                return order->getOrderId() == item.params.orderId;
            });
        if ( found == m_Data.end() ) {
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
}

void TradeHistoryStorage::remove(const Market::MarketId &id) noexcept
{
    LOG_TRACE_TIME();
    QReadLocker locked(&lock);
    m_Filters.removeAll(id);
    for (auto item = m_Data.begin(); item != m_Data.end();) {
        if ( item->get()->getMarket()->getId() == id )
            item = m_Data.erase(item);
        else
            ++item;
    }
}
