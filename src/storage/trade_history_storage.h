#ifndef TRADE_HISTORY_STORAGE_H
#define TRADE_HISTORY_STORAGE_H

#include <api/i_exchange_api_entry.h>
#include <QReadWriteLock>
class IExchangeAPI;

class TradeHistoryStorage final: public QObject
{
    Q_OBJECT
public:
    explicit TradeHistoryStorage(IExchangeAPI *parent = nullptr) noexcept;
    explicit TradeHistoryStorage(const TradeHistoryStorage &o) noexcept = delete;
    TradeHistoryStorage& operator=(const TradeHistoryStorage &o) noexcept = delete;
    Order::ConstContainer filter(const Market::MarketId &marketId) const noexcept;
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

#endif // TRADE_HISTORY_STORAGE_H
