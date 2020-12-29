#ifndef ABSTRACTANALYTICSALGORITHM_H
#define ABSTRACTANALYTICSALGORITHM_H
#include <QRunnable>
#include <common/enums.h>
#include <base/market.h>
#include <base/order.h>

class ITradeAlgorithm;

#pragma pack (push, 1)
class AbstractAnalyticsAlgorithm : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit AbstractAnalyticsAlgorithm(QObject *parent = nullptr) noexcept;
    virtual ~AbstractAnalyticsAlgorithm() noexcept override = default;
    using Ptr = QSharedPointer<AbstractAnalyticsAlgorithm>;
    static Ptr createAlgo(const AnalyticsAlgoType type) noexcept;

    void setTradeAlgo(const QSharedPointer<ITradeAlgorithm> &algo) noexcept;

    QSharedPointer<ITradeAlgorithm> tradeAlgo() const;
public:
    void run() noexcept override final;
public slots:
    virtual bool start() = 0;
    //TODO: пока не обрабатывается
    virtual bool stop() = 0;
signals:
    void ready();
    void finished();
    void updateMarketConfig(const Market::MarketId &marketId, const Market::UserConfig &config);
    void cancelOrderId(const Market::MarketId &marketId, const Order::OrderId &id);
    QT_DEPRECATED void submitSellOrderToBuyPrice(const Market::MarketId &id, const qreal &price, const qreal &currencyVolume);
protected:
    QSharedPointer<ITradeAlgorithm> m_TradeAlgo = {};
};
#pragma pack (pop)
#endif // ABSTRACTANALYTICSALGORITHM_H
