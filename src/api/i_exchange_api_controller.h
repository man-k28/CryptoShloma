#ifndef I_EXCHANGE_API_CONTROLLER_H
#define I_EXCHANGE_API_CONTROLLER_H
#include "base/order.h"
class IExchangeAPI;

class IExchangeApiController : public QObject
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<IExchangeApiController>;
    using ConstPtr = QSharedPointer<const IExchangeApiController>;
    using MarketsGroup = QHash<Market::MarketId, Market::UserConfig>;
public:
    explicit IExchangeApiController(IExchangeAPI *api) noexcept
        : QObject()
        , m_api(api) {}
    explicit IExchangeApiController(const IExchangeApiController &o) noexcept = delete;
    IExchangeApiController& operator=(const IExchangeApiController &o) noexcept = delete;
    ~IExchangeApiController() override = default;
public slots:
    virtual void run() noexcept = 0;
    virtual void appendMarket(const Market::MarketId &marketId,
                              const Market::UserConfig &config) = 0;
    virtual void appendMarketsGroup(const MarketsGroup &group) = 0;
    virtual void removeMarket(const Market::MarketId &marketId) = 0;
    virtual void cancelAllOrders() = 0;
    virtual void cancelOrder(const Market::MarketId &marketId,
                             const Order::OrderId &orderId) = 0;
    virtual void submitOrder(const Market::MarketId &marketId,
                             const qreal &price,
                             const qreal &currencyVolume) = 0;
    virtual void dropCoin(const Currency::CurrencyId &currencyId) = 0;
signals:
    void appendedMarket(const Market::MarketId &);
    void removedMarket(const Market::MarketId &);
    void updateMarketConfig(const Market::MarketId &id, const Market::UserConfig &config);
protected:
    IExchangeAPI * m_api;
};

#endif // I_EXCHANGE_API_CONTROLLER_H
