#ifndef OPEN_ORDERS_STORAGE_H
#define OPEN_ORDERS_STORAGE_H

#include <api/i_exchange_api_entry.h>
#include <QReadWriteLock>
class IExchangeAPI;

class OpenOrdersStorage final : public QObject
{
    Q_OBJECT
public:
    explicit OpenOrdersStorage(IExchangeAPI *parent) noexcept;
    explicit OpenOrdersStorage(const OpenOrdersStorage &o) noexcept = delete;
    OpenOrdersStorage& operator=(const OpenOrdersStorage &o) noexcept = delete;
    Order::ConstContainer filter(const Market::MarketId &marketId, const Order::OrderType orderType) const noexcept;
    qint32 count() const noexcept;
public:
    Order::ConstContainer ordersByMarketId(const Market::MarketId &id) const noexcept;
public slots:
    void merge(const QList<exchange::OrderEntry> &data) noexcept;
    void remove(const QList<Order::OrderId> &data) noexcept;
private:
    Order::Container m_Data{};
    IExchangeAPI * const m_api;
    QReadWriteLock lock{};
};

#endif // OPEN_ORDERS_STORAGE_H
