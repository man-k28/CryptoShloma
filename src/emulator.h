#ifndef EMULATOR_H
#define EMULATOR_H

#include <api/i_exchange_api_entry.h>

class IExchangeAPI;

class Emulator : public QObject
{
    Q_OBJECT
public:
    explicit Emulator(IExchangeAPI *parent) noexcept;

public slots:
    void loadMarketOrders(const QUrl &url) noexcept;

    void submitOrder(const Market::MarketId &marketId,
                     const Order::OrderId &orderId,
                     const Order::OrderType side,
                     const qreal &price,
                     const qreal &baseVolume,
                     const qreal &currencyVolume = 0.) noexcept;
    void cancelOrder(const Order::OrderId &orderId = Order::OrderId(0),
                     const Market::MarketId &marketId = Market::MarketId()) noexcept;
    void sellOrder(const Market::MarketId &marketId,
                   const Order::OrderId &orderId,
                   const qreal &price,
                   const qreal &baseVolume,
                   const qreal &currencyVolume) noexcept;
signals:
    void loadedMarketOrders(const QList<exchange::OrderEntry> &list);
    void loadedUserOpenOrders(const QList<exchange::OrderEntry> &list);
    void loadedBalance(const QList<exchange::BalanceEntry> &list);
    void loadedUserTradeHistory(const QList<exchange::OrderEntry> &list);
    void loadedCanceledOrders(const QList<Order::OrderId> &ids);
};

#endif // EMULATOR_H
