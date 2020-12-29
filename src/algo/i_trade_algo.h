#ifndef ITRADEALGORITHM_H
#define ITRADEALGORITHM_H
#include <QRunnable>
#include <common/enums.h>
#include <base/market.h>

class IExchangeAPI;

#pragma pack (push, 1)
class ITradeAlgorithm : public QObject, public QRunnable
{
    Q_OBJECT
public:
    using ConstPtr = QSharedPointer<const ITradeAlgorithm>;
    using Ptr = QSharedPointer<ITradeAlgorithm>;
public:
    explicit ITradeAlgorithm(QObject *parent = nullptr) noexcept;
    virtual ~ITradeAlgorithm() noexcept override = default;

    static Ptr createAlgo(const AlgoType type, const Exchange::Type exchType) noexcept;
    void setApi(const QSharedPointer<IExchangeAPI> &api) noexcept;

    inline const QSharedPointer<IExchangeAPI> &api() const noexcept
    {
        return m_Api;
    }
public:
    void run() noexcept override final;
public slots:
    void init() noexcept;
    virtual bool start() = 0;
    //TODO: пока не обрабатывается
    virtual bool stop() = 0;
    virtual bool restart() = 0;
    virtual void reconfigure() = 0;
    virtual void removeMarket(const Market::MarketId &) {}
signals:
    void updateMarketConfig(const Market::MarketId &marketId, const Market::UserConfig &config);
    void ready();
    void finished();
private:
    QSharedPointer<IExchangeAPI> m_Api = {};
};
#pragma pack (pop)
#endif // ITRADEALGORITHM_H
