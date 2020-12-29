#include "i_analytics_algo.h"
#include "i_trade_algo.h"
#include <api/i_exchange_api.h>

#include "trend_searcher.h"
#include <storage/markets_storage.h>

AbstractAnalyticsAlgorithm::AbstractAnalyticsAlgorithm(QObject *parent) noexcept :
    QObject(parent)
{
    connect(this, &AbstractAnalyticsAlgorithm::ready, this, &AbstractAnalyticsAlgorithm::start);
}

AbstractAnalyticsAlgorithm::Ptr AbstractAnalyticsAlgorithm::createAlgo(const AnalyticsAlgoType type) noexcept
{
    switch (type) {
    case TrendSearcherAlgo:
        return QSharedPointer<TrendSearcher>::create();
    }
    return nullptr;
}

void AbstractAnalyticsAlgorithm::run() noexcept
{
    emit ready();
}

void AbstractAnalyticsAlgorithm::setTradeAlgo(const QSharedPointer<ITradeAlgorithm> &algo) noexcept
{
    m_TradeAlgo = algo;

    auto marketsStorage = m_TradeAlgo->api()->marketsStorage();
    assert(marketsStorage);
    connect(this, &AbstractAnalyticsAlgorithm::updateMarketConfig, marketsStorage, &MarketsStorage::updateMarketUserConfig, Qt::QueuedConnection);

    connect(this, &AbstractAnalyticsAlgorithm::cancelOrderId, m_TradeAlgo->api()->controller().get(), &IExchangeApiController::cancelOrder);
    connect(this, &AbstractAnalyticsAlgorithm::submitSellOrderToBuyPrice, m_TradeAlgo->api()->controller().get(), &IExchangeApiController::submitOrder);
}

QSharedPointer<ITradeAlgorithm> AbstractAnalyticsAlgorithm::tradeAlgo() const
{
    return m_TradeAlgo;
}
