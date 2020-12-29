#ifndef MARKET_ORDERS_STORAGE_H
#define MARKET_ORDERS_STORAGE_H

#include <api/i_exchange_api_entry.h>
#include <QReadWriteLock>
class IExchangeAPI;

class MarketOrdersStorage final : public QObject
{
    Q_OBJECT
public:
    explicit MarketOrdersStorage(IExchangeAPI *parent) noexcept;
    explicit MarketOrdersStorage(const MarketOrdersStorage &o) noexcept = delete;
    MarketOrdersStorage& operator=(const MarketOrdersStorage &o) noexcept = delete;
    Order::ConstContainer filter(const Market::MarketId &marketId, const Order::OrderType orderType) const noexcept;
public slots:
    void insert(const Market::MarketId &id) noexcept;
    void merge(const QList<exchange::OrderEntry> &data) noexcept;
    void remove(const Market::MarketId &id) noexcept;
private:
    Order::Container m_Data{};
    QList<Market::MarketId> m_Filters{};
    IExchangeAPI * const m_api;
    QReadWriteLock lock{};
};

#endif // MARKET_ORDERS_STORAGE_H
