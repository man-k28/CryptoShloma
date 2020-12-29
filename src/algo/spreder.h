#ifndef SPREDER_H
#define SPREDER_H
#include "i_trade_algo.h"
#include <base/order.h>

class Spreder final: public ITradeAlgorithm
{
    Q_OBJECT
public:
    explicit Spreder(QObject *parent = nullptr) noexcept;
    ~Spreder() override = default;
private slots:
    bool start() noexcept override final;
    bool stop() noexcept override final;
    bool restart() noexcept override final;
    void reconfigure() noexcept override final;
    void timerEvent(QTimerEvent *event) noexcept final;
private:
    void startSubmitOrders(const Market::ConstPtr &market, const bool sellMode);
    void dropCoin(const Market::ConstPtr &market);

    struct Historydata {
        qreal amountBtc{0.};
        qreal rate{0.};
        qreal fee{0.};
        qreal amountCurrency{0.};
    };

    qreal calculateProfit(const qreal &bid, const qreal &ask, const qreal &fee) const;
    qreal calculateStopLoss(const qreal &oldBid,
                            const qreal &oldFee,
                            const qreal &oldAmountBtc,
                            const qreal &fee,
                            const qreal &oldAmountCurrency,
                            const qreal &pairProfit) const;
    qreal calculateBuyIn(const qreal &oldBid,
                         const qreal &oldFee,
                         const qreal &fee,
                         const qreal &oldAmountCurrency,
                         const qreal &pairProfit,
                         const qreal &step) const;

    Historydata findPreviousBuyOrders(const Order::ConstContainer &container) const noexcept;
    QPair<qreal,qreal> getStepMerchant(const qreal &trend) const;
    void searchAndCancelOrders(const Order::ConstContainer &container) noexcept;
};

#endif // SPREDER_H
