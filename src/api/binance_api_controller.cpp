#include "binance_api_controller.h"
#include "i_exchange_api.h"
#include <storage/markets_storage.h>
#include <storage/market_orders_storage.h>
#include <storage/open_orders_storage.h>
#include <storage/trade_history_storage.h>
#include <common/settings_manager.h>
#include <QTimerEvent>

BinanceAPIController::BinanceAPIController(IExchangeAPI *api) noexcept
    : IExchangeApiController(api)
{
    connect(m_api, &IExchangeAPI::disconnectedPublicApi, this, [this]() {
        if ( !SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
            m_api->listMarketInfo();
            m_api->listMarketData();
            m_api->start();
        }
    });

    connect(m_api, &IExchangeAPI::disconnectedPrivateApi, this, [this]() {
        if ( !SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
            m_api->serverTime();
            m_api->userAuth();
            const auto &pairs = m_api->marketsStorage()->filterTradingPairs();
            for ( const auto &market : pairs ) {
                m_api->userTradeHistory(market->getId());
                m_api->userOpenOrders(market->getId());
            }
            m_api->userBalance();
        }
    });
}

BinanceAPIController::~BinanceAPIController() noexcept
{
    if ( m_marketInfoTimerId ) {
        killTimer(m_marketInfoTimerId);
        m_marketInfoTimerId = 0;
    }
}

void BinanceAPIController::run() noexcept
{
    m_api->listMarketInfo();
    m_api->listMarketData();
    if ( !SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
        m_api->serverTime();
        m_api->userAuth();
        m_api->userBalance();
        m_api->start();
        const qint32 timeout = 30 * 60 * CONST_MIN_TIMEOUT;
        if (m_marketInfoTimerId == 0)
            m_marketInfoTimerId = startTimer(timeout);
    } else {
        m_api->userBalance();
        m_api->userTradeHistory(Market::MarketId());
    }
    m_api->userOpenOrders(Market::MarketId());
}

void BinanceAPIController::appendMarket(const Market::MarketId &marketId, const Market::UserConfig &config)
{
    emit appendedMarket(marketId);
    emit updateMarketConfig(marketId, config); //BUG: если прилетает от сигнала загрузки из БД, то модель то не обновилась, потому что обновляем роли из гуя (это касается обновления настроек на ходу)
    if ( !SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
        m_api->listMarketOrders(marketId);
        m_api->userTradeHistory(marketId);
        m_api->userOpenOrders(marketId);
    }
}

void BinanceAPIController::appendMarketsGroup(const IExchangeApiController::MarketsGroup &group)
{
    QList<Market::MarketId> marketIds;

    IExchangeApiController::MarketsGroup::const_iterator it = group.cbegin();
    while ( it != group.cend() ) {
        const auto &marketId = it.key();
        emit appendedMarket(marketId);
        emit updateMarketConfig(marketId, it.value()); //BUG: если прилетает от сигнала загрузки из БД, то модель то не обновилась, потому что обновляем роли из гуя (это касается обновления настроек на ходу)
        if ( !SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
            m_api->userTradeHistory(marketId);
            m_api->userOpenOrders(marketId);
        }

        marketIds << marketId;
        ++it;
    }
    if ( !SettingsManager::get(SettingsManager::EnableTestData).toBool() )
        m_api->listMarketOrderGroups(marketIds);
}

void BinanceAPIController::removeMarket(const Market::MarketId &marketId)
{
    m_api->removeListMarketOrders(marketId);
    emit removedMarket(marketId);
    const auto &market = m_api->marketsStorage()->findMarketById(marketId); //TODO: отказаться от поиска в пользу передачи указателя из гуя
    if ( !market.isNull() ) {
        Market::UserConfig config = market->getUserConfig();
        config.lastRateUpdateTime = QDateTime::currentDateTimeUtc();
        config.isTradable = false;
        config.isVolumeTrading = false;
        config.currentTrendRate = 0.;
        config.minTrend = 0.;
        emit updateMarketConfig(marketId, config);
        if ( !SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
            const auto &openOrdersList = m_api->openOrdersStorage()->ordersByMarketId(marketId);
            for (const auto &order: openOrdersList) {
                if ( order->getType() == Order::Buy )
                    m_api->cancelOrder(IExchangeAPI::OrderId, order->getOrderId());
            }
        }
    }
}

void BinanceAPIController::cancelAllOrders()
{
    m_api->cancelOrder(IExchangeAPI::All);
}

void BinanceAPIController::cancelOrder(const Market::MarketId &marketId, const Order::OrderId &orderId)
{
    m_api->cancelOrder(IExchangeAPI::OrderId, orderId, marketId);
}

void BinanceAPIController::submitOrder(const Market::MarketId &marketId, const qreal &price, const qreal &currencyVolume)
{
    m_api->submitOrder(marketId, Order::Sell, price, price * currencyVolume, currencyVolume);
}

void BinanceAPIController::dropCoin(const Currency::CurrencyId &currencyId)
{
    const auto &market = m_api->marketsStorage()->findMarketByCurrencyId(currencyId);
    removeMarket(market->getId());
    m_api->marketOrdersStorage()->remove(market->getId());
}

void BinanceAPIController::timerEvent(QTimerEvent *event) noexcept
{
    if ( event->timerId() == m_marketInfoTimerId)
        m_api->listMarketInfo();

//        for(const auto &market : m_api->marketsStorage()->filterTradingPairs()) {
//            const auto &openOrdersList = m_api->openOrdersStorage()->ordersByMarketId(market->getId());
//            for (const auto &order: openOrdersList) {
//                if ( order->getType() == Order::Buy )
//                    m_api->cancelOrder(IExchangeAPI::OrderId, order->getOrderId());
//            }
//        }

//            QList<Market::MarketId> group;
//            for (const auto &market : m_api->marketsStorage()->filterTradingPairs()) {
//                if ( !market.isNull() ) {
//                    if ( !(group.count() % 5) && group.count() ) {
//                        api()->listMarketOrderGroups(group);
//                        group.clear();
//                    }
//                    group << val;
//                    if ( !market.isNull() && !market->getPauseMode() )
//                        startSubmitOrders(market, market->getSellMode());
//                }
//            }
//            if ( !group.isEmpty() )
//                api()->listMarketOrderGroups(group);
}
