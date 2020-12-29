#include "i_exchange_api.h"
#include "api/binance_api.h"
//#include <QThread>
#include <cmath>
#include <QTimerEvent>
#include "common/settings_manager.h"
#include "Logger.h"
#include "storage/balances_storage.h"
#include "storage/market_orders_storage.h"
#include "storage/markets_storage.h"
#include "storage/open_orders_storage.h"
#include "storage/trade_history_storage.h"
#include "temporary_orders_table.h"
#include <cryptoshloma.h>
#include <emulator.h>

IExchangeAPI::IExchangeAPI(QObject *parent) noexcept
    : QObject(parent)
    , m_marketsStorage(new MarketsStorage(this))
    , m_marketOrdersStorage(new MarketOrdersStorage(this))
    , m_openOrdersStorage(new OpenOrdersStorage(this))
    , m_balancesStorage(new BalancesStorage(this))
    , m_tradeHistoryStorage(new TradeHistoryStorage(this))
    , m_tempOrdersTable(new TemporaryOrdersTable(this))
    , m_emulator(new Emulator(this))
{
    connect(this, &IExchangeAPI::receiveCurrencies, m_marketsStorage, &MarketsStorage::mergeCurrencies, Qt::QueuedConnection);
    connect(this, &IExchangeAPI::receiveMarketInfo, m_marketsStorage, &MarketsStorage::mergeMarketInfo, Qt::QueuedConnection);
    connect(this, &IExchangeAPI::receiveMarketData, m_marketsStorage, &MarketsStorage::mergeMarketData, Qt::QueuedConnection);
    connect(this, &IExchangeAPI::receiveMarketOrders, m_marketOrdersStorage, &MarketOrdersStorage::merge, Qt::QueuedConnection);
    connect(m_emulator, &Emulator::loadedMarketOrders, m_marketOrdersStorage, &MarketOrdersStorage::merge, Qt::QueuedConnection);
    connect(this, &IExchangeAPI::receiveUserOpenOrders, m_openOrdersStorage, &OpenOrdersStorage::merge, Qt::QueuedConnection);
    connect(m_emulator, &Emulator::loadedUserOpenOrders, m_openOrdersStorage, &OpenOrdersStorage::merge, Qt::QueuedConnection);
    connect(this, &IExchangeAPI::receiveCanceledOrders, m_openOrdersStorage, &OpenOrdersStorage::remove);
    connect(m_emulator, &Emulator::loadedCanceledOrders, m_openOrdersStorage, &OpenOrdersStorage::remove);
    connect(this, &IExchangeAPI::receiveBalance, m_balancesStorage, &BalancesStorage::merge, Qt::QueuedConnection);
    connect(m_emulator, &Emulator::loadedBalance, m_balancesStorage, &BalancesStorage::merge, Qt::QueuedConnection);
    connect(this, &IExchangeAPI::receiveUserTradeHistory, m_tradeHistoryStorage, &TradeHistoryStorage::merge, Qt::QueuedConnection);
    connect(m_emulator, &Emulator::loadedUserTradeHistory, m_tradeHistoryStorage, &TradeHistoryStorage::merge, Qt::QueuedConnection);
    connect(this, &IExchangeAPI::emulSubmitOrder, m_emulator, &Emulator::submitOrder);
    connect(this, &IExchangeAPI::emulCanceLorder, m_emulator, &Emulator::cancelOrder);

    connect(m_marketsStorage, &MarketsStorage::removeTradePairIdFromTrading, m_marketOrdersStorage, &MarketOrdersStorage::remove );

//    m_ModelsThread = new QThread();
//    connect(m_ModelsThread, &QThread::finished, m_ModelsThread, &QThread::deleteLater);
//    connect(m_ModelsThread, &QThread::finished, m_marketsStorage, &QObject::deleteLater);
//    connect(m_ModelsThread, &QThread::finished, m_marketOrdersStorage, &QObject::deleteLater);
//    connect(m_ModelsThread, &QThread::finished, m_openOrdersStorage, &QObject::deleteLater);
//    connect(m_ModelsThread, &QThread::finished, m_balancesStorage, &QObject::deleteLater);
//    connect(m_ModelsThread, &QThread::finished, m_tradeHistoryStorage, &QObject::deleteLater);
//    m_marketsStorage->moveToThread(m_ModelsThread);
//    m_marketOrdersStorage->moveToThread(m_ModelsThread);
//    m_openOrdersStorage->moveToThread(m_ModelsThread);
//    m_balancesStorage->moveToThread(m_ModelsThread);
//    m_tradeHistoryStorage->moveToThread(m_ModelsThread);
}

IExchangeAPI::~IExchangeAPI()
{
//    m_ModelsThread->quit();
//    m_ModelsThread->wait();
}

IExchangeAPI::Ptr IExchangeAPI::createExchangeApi(const Exchange::Type type) noexcept
{
    switch (type) {
    case Exchange::Type::Binance:
        return QSharedPointer<BinanceAPI>::create();
    default:
        return nullptr;
    }
}

Emulator *IExchangeAPI::getEmulator() noexcept
{
    return m_emulator;
}

const IExchangeApiController::ConstPtr IExchangeAPI::controller() const noexcept
{
    return m_controller;
}

TemporaryOrdersTable *IExchangeAPI::tempOrdersTable() const noexcept
{
    return  m_tempOrdersTable;
}

void IExchangeAPI::run() noexcept
{
    //TODO: убрать привязку к этому событию, сделать отдельное
//    connect(m_ModelsThread, &QThread::started, m_controller.get(), &IExchangeApiController::run);
    connect(m_controller.get(), &IExchangeApiController::removedMarket, m_marketOrdersStorage, &MarketOrdersStorage::remove);
    connect(m_controller.get(), &IExchangeApiController::appendedMarket, m_marketOrdersStorage, &MarketOrdersStorage::insert);
    connect(m_controller.get(), &IExchangeApiController::removedMarket, m_tradeHistoryStorage, &TradeHistoryStorage::remove);
    connect(m_controller.get(), &IExchangeApiController::appendedMarket, m_tradeHistoryStorage, &TradeHistoryStorage::insert);
//    m_ModelsThread->start();
//    m_controller->run();
}

void IExchangeAPI::calculateSpreadForMarket(Market::Config &config) const noexcept
{
    const qreal &tradeSummBaseCurrency = SettingsManager::get(SettingsManager::TradeBaseSumm).toReal();
    const qreal &difference = SettingsManager::get(SettingsManager::TradeDifference).toReal();
    const qreal &askPrice = config.askPrice - difference; // Sell price
    const qreal &bidPrice = config.bidPrice + difference; //Buy price
    const qreal &tradeFee = 1 - config.tradeFee; //%

    const qreal &startBuyCurr = tradeSummBaseCurrency / bidPrice  * tradeFee;
    const qreal &startSellCurr = startBuyCurr * askPrice * tradeFee;

    config.spread = std::floor( (startSellCurr - tradeSummBaseCurrency) * std::pow(10,8)) * PRECISSION;
}

void IExchangeAPI::setOffline() noexcept
{
    if ( m_isOnline ) {
        m_isOnline = false;
        emit isOnlineChanged();
    }
}

void IExchangeAPI::setOnline() noexcept
{
    if ( !m_isOnline ) {
        m_isOnline = true;
        emit isOnlineChanged();
    }
}

IExchangeApiController *IExchangeAPI::getController() const noexcept
{
    return m_controller.get();
}
