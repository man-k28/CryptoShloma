#include "i_trade_algo.h"
#include <api/i_exchange_api.h>
#include "hanukkah.h"
#include "spreder.h"
#include <storage/markets_storage.h>
#include <QThreadPool>

ITradeAlgorithm::ITradeAlgorithm(QObject *parent) noexcept
    : QObject(parent)
{
    connect(this, &ITradeAlgorithm::ready, this, &ITradeAlgorithm::start, Qt::QueuedConnection);
}

ITradeAlgorithm::Ptr ITradeAlgorithm::createAlgo(const AlgoType type, const Exchange::Type exchType) noexcept
{
    auto api = IExchangeAPI::createExchangeApi(exchType);
    api->setAutoDelete(false);
    QSharedPointer<ITradeAlgorithm> algo;
    switch (type) {
    case SprederAlgo:
        algo = QSharedPointer<Spreder>::create();
        break;
    case HanukkahAlgo:
        algo = QSharedPointer<Hanukkah>::create();
        break;
    }

//    QObject::connect(api.get(), &IExchangeAPI::destroyed, algo.get(), [&algo](){algo.clear();});
    algo->setApi(api);
    return algo;
}

void ITradeAlgorithm::run() noexcept
{
    QThreadPool::globalInstance()->start(m_Api.get());
    connect(this, &ITradeAlgorithm::ready, m_Api->controller().get(), &IExchangeApiController::run);
    emit ready();
}

void ITradeAlgorithm::setApi(const QSharedPointer<IExchangeAPI> &api) noexcept
{
    m_Api = api;
    auto marketsStorage = api->marketsStorage();
    assert(marketsStorage);
    connect(m_Api->controller().get(), &IExchangeApiController::updateMarketConfig, marketsStorage, &MarketsStorage::updateMarketUserConfig, Qt::QueuedConnection);
    connect(m_Api->controller().get(), &IExchangeApiController::removedMarket, this, &ITradeAlgorithm::removeMarket);
    connect(this, &ITradeAlgorithm::updateMarketConfig, api->marketsStorage(), &MarketsStorage::updateMarketUserConfig, Qt::QueuedConnection);
}

void ITradeAlgorithm::init() noexcept
{
    connect(m_Api->marketsStorage(), &MarketsStorage::removeTradePairIdFromTrading, m_Api->controller().get(), &IExchangeApiController::removeMarket);
}
