#include "open_orders_storage.h"
#include <api/i_exchange_api.h>
#include <Logger.h>
#include "markets_storage.h"
#include <QWriteLocker>

OpenOrdersStorage::OpenOrdersStorage(IExchangeAPI *parent) noexcept
    : QObject()
    , m_api(parent)
{
}

Order::ConstContainer OpenOrdersStorage::filter(const Market::MarketId &marketId, const Order::OrderType orderType) const noexcept
{
    Order::ConstContainer filtered;

    std::copy_if(m_Data.begin(), m_Data.end(),std::back_inserter(filtered), [&marketId, orderType](const Order::Ptr &order){
        return order->getMarket()->getId() == marketId && order->getType() == orderType;
    });
    std::sort(filtered.begin(), filtered.end(),[](const Order::ConstPtr &leftOrder, const Order::ConstPtr &rightOrder){
        return leftOrder->getTimeStamp() > rightOrder->getTimeStamp();
    });
    return filtered;
}

qint32 OpenOrdersStorage::count() const noexcept
{
    return m_Data.count();
}

Order::ConstContainer OpenOrdersStorage::ordersByMarketId(const Market::MarketId &id) const noexcept
{
    Order::ConstContainer ordersList;
    for(const auto &val : m_Data) {
        if ( val->getMarket()->getId() == id ) {
            ordersList << val;
        }
    }

    return ordersList;
}

void OpenOrdersStorage::merge(const QList<exchange::OrderEntry> &data) noexcept
{
    LOG_TRACE_TIME();
    QReadLocker locked(&lock);
    Order::Container filtered;
    for (const auto &item : data) {
        std::copy_if(m_Data.begin(), m_Data.end(), std::back_inserter(filtered), [&item, &filtered](const Order::Ptr &order){
            return item.marketId == order->getMarket()->getId() && !filtered.contains(order);
        });
    }

    for (auto item = filtered.begin(); item != filtered.end();) {
        auto found = std::find_if(data.cbegin(), data.cend(),
            [ &item ]( const exchange::OrderEntry &order ) noexcept
            {
                return item->get()->getOrderId() == order.params.orderId ;
            });
        if ( found == data.cend() )
            item = filtered.erase(item);
        else
            ++item;
    }

    if ( filtered.count() ) {
        for (auto item = filtered.begin(); item != filtered.end();++item) {
            m_Data.removeOne(*item);
        }
    }

    for (const auto &item : data) {
        auto found = std::find_if(m_Data.begin(), m_Data.end(),
            [ &item ]( const decltype( m_Data )::value_type &order ) noexcept
            {
                return order->getOrderId() == item.params.orderId;
            });
        if ( found == m_Data.end() ) {
            auto market = m_api->marketsStorage()->findMarketById(item.marketId);
            if ( !market.isNull() ) {
                auto order = Order::Ptr::create(item.params);
                order->setMarket(market);
                m_Data.append(order);
            }
        } else {
            if ( item.params.updatedTimeStamp >= found->get()->getUpdatedTimeStamp() )
                found->get()->setConfig(item.params);
        }
    }
}

void OpenOrdersStorage::remove(const QList<Order::OrderId> &data) noexcept
{
    LOG_TRACE_TIME();
    QReadLocker locked(&lock);
    for (auto item = m_Data.begin(); item != m_Data.end();) {
        auto found = std::find_if(data.cbegin(), data.cend(),
            [ &item ]( const Order::OrderId &id ) noexcept
            {
                return item->get()->getOrderId() == id;
            });
        if ( found != data.cend() )
            item = m_Data.erase(item);
        else
            ++item;
    }
}
