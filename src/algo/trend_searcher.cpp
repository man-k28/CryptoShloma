#include "trend_searcher.h"
#include <QTimerEvent>
#include <cmath>
#include <common/constants.h>
#include "i_trade_algo.h"
#include <api/i_exchange_api.h>
#include <storage/markets_storage.h>
#include <storage/market_orders_storage.h>
#include <storage/trade_history_storage.h>

TrendSearcher::TrendSearcher(QObject *parent) noexcept :
    AbstractAnalyticsAlgorithm(parent)
{
}

void TrendSearcher::timerEvent(QTimerEvent *event) noexcept
{
    if ( event->timerId() == m_TimerId && !m_TradeAlgo.isNull() && !m_TradeAlgo->api().isNull() ) {
        const auto &tradingPairsList = m_TradeAlgo->api()->marketsStorage()->filterTradingPairs();
        for( const auto &market : tradingPairsList ) {
            const auto &marketId = market->getId();
            const auto &tradeHistoryFiltered = m_TradeAlgo->api()->tradeHistoryStorage()->filter(marketId);

            if ( !tradeHistoryFiltered.count() )
                return;
            const auto &order = tradeHistoryFiltered.first();

            if ( !order.isNull() ) {
                //NOTE: Если последний ордер был на покупку
                if (order->getType() == Order::Buy) {
                    auto metaConfig = market->getMetaConfig();
                    const QDateTime &cycleTimeStart = QDateTime::fromString(metaConfig.value(QStringLiteral("cycleTimeStart")).toString(), Qt::ISODate);
                    if ( cycleTimeStart.isNull() || !cycleTimeStart.isValid())
                        return;
                    const qreal &firstSellPrice = findFirstSellPrice(tradeHistoryFiltered, cycleTimeStart);
                    const auto &sellOrdersFiltered = m_TradeAlgo->api()->marketOrdersStorage()->filter(marketId, Order::OrderType::Sell);

                    if ( sellOrdersFiltered.isEmpty() )
                        return;
                    const auto &sellOrder = sellOrdersFiltered.first();
                    if ( !sellOrder.isNull() ) {
                        const qreal &topSellPrice = sellOrder->getPrice();
                        //NOTE: рассчитываем разницу курса
                        const qreal &calcCurrentRatePercentage = calcPerc(firstSellPrice, topSellPrice);
                        //NOTE: обновляем % падения
                        const qreal &previousMinRate = market->getMinTrend();
                        if ( !qFuzzyCompare(market->getCurrentTrendRate(), calcCurrentRatePercentage) ) {
                            Market::UserConfig config = market->getUserConfig();
                            config.currentTrendRate = calcCurrentRatePercentage;
                            config.lastRateUpdateTime = QDateTime::currentDateTimeUtc();
                            if ( calcCurrentRatePercentage < previousMinRate ) //FIXME: double compare
                                config.minTrend = calcCurrentRatePercentage;
                            emit updateMarketConfig(marketId, config);
                        }
                    }
                } else {
                    if ( !qFuzzyIsNull(market->getCurrentTrendRate()) ) {
                        Market::UserConfig config = market->getUserConfig();
                        config.currentTrendRate = 0.;
                        config.minTrend = 0.;
                        config.lastRateUpdateTime = QDateTime::currentDateTimeUtc();
                        emit updateMarketConfig(marketId, config);
                    }
                }
            }
        }
    }
}

bool TrendSearcher::start() noexcept
{
    if ( m_TimerId == 0)
        m_TimerId = startTimer(CONST_MIN_TIMEOUT);
    return true;
}

bool TrendSearcher::stop() noexcept
{
    if ( m_TimerId) {
        killTimer(m_TimerId);
        m_TimerId = 0;
    }
    return true;
}

qreal TrendSearcher::calcPerc(const qreal &a, const qreal &b) const noexcept
{
    if ( qFuzzyIsNull(a) )
        return 0.;

    if ( a < 0. && b < 0.) { //FIXME: double compare
        if ( std::fabs(a) < std::fabs(b) ) {//FIXME: double compare
            return (((a - b) / a) * 100.);
        } else if ( std::fabs(a) > std::fabs(b) ) {
            return -(( b - a) / a) * 100.;
        } else
            return 0.;
    } else {
        if ( a > b )
            return -(((a - b) / a) * 100.);
        else if ( a < b ) //FIXME: double compare
            return std::fabs((( b - a) / a) * 100.);
        else
            return 0.;
    }
}

qreal TrendSearcher::findFirstSellPrice(const Order::ConstContainer &model, const QDateTime &cycleTimeStart) noexcept
{
    qreal ret = 0.;
    bool find = false;

    for (const auto &order : model) {
        if ( !order.isNull() ) {
            if ( (order->getType() == Order::Buy)
                  && (order->getUpdatedTimeStamp() >= cycleTimeStart)
            ) {
                find = true;
                ret = order->getPrice();
            }

            if ( order->getType() == Order::Sell && find )
                break;
        }
    }

    return std::floor(ret * std::pow(10,8)) * PRECISSION;
}
