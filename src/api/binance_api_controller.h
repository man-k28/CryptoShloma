#ifndef BINANCE_API_CONTROLLER_H
#define BINANCE_API_CONTROLLER_H
#include "i_exchange_api_controller.h"

class BinanceAPIController final : public IExchangeApiController
{
    Q_OBJECT
public:
    explicit BinanceAPIController(IExchangeAPI *api) noexcept;
    ~BinanceAPIController() noexcept override;
private slots:
    void run() noexcept final;
    void timerEvent(QTimerEvent *event) noexcept override final;
    void appendMarket(const Market::MarketId &marketId,
                      const Market::UserConfig &config) override final;
    void appendMarketsGroup(const MarketsGroup &group) override final;
    void removeMarket(const Market::MarketId &marketId) override final;
    void cancelAllOrders() override final;
    void cancelOrder(const Market::MarketId &marketId,
                     const Order::OrderId &orderId) override final;
    void submitOrder(const Market::MarketId &marketId,
                             const qreal &price,
                             const qreal &currencyVolume) override final;
    void dropCoin(const Currency::CurrencyId &currencyId) override final;
private:
    qint32 m_marketInfoTimerId = 0;
};

#endif // BINANCE_API_CONTROLLER_H
