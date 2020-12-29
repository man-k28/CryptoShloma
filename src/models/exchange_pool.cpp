#include "exchange_pool.h"
#include <crud.h>
#include <storage/markets_storage.h>
#include <storage/market_orders_storage.h>
#include <QSqlRecord>
#include <QThreadPool>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <Models>
#include <cryptoshloma.h>
#include <emulator.h>

ExchangePool::ExchangePool(QObject *parent) noexcept
    : QAbstractListModel(parent)
{
}

ExchangePool::~ExchangePool() noexcept
{
    for (auto entry : m_storage)
        saveState(entry->algo()->api()->type()); //TODO: сохранение состояния должна быть в другом месте
}

int ExchangePool::rowCount(const QModelIndex &parent) const noexcept
{
    return parent.isValid() ? 0 : m_storage.count();
}

QVariant ExchangePool::data(const QModelIndex &index, int role) const noexcept
{
    Q_UNUSED(role)
    if (!index.isValid()
            || ( index.row() < 0 )
            || ( index.row() >= m_storage.count() )
    )
        return QVariant();

    const auto &exch = m_storage.at(index.row());
    if (!exch.isNull())
        return QVariant::fromValue(exch.get());
    else
        return {};
}

QHash<int, QByteArray> ExchangePool::roleNames() const noexcept
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[Exchange] = "exchange";
    return roles;
}

void ExchangePool::append(const Exchange::Type type) noexcept
{
    if ( findByType(type) == m_storage.end() )
        createEntry(type);
    else
        reloadSettings();
}

void ExchangePool::remove(const Exchange::Type type, bool doSaveState) noexcept
{
    auto found = findByType(type);
    if ( found != m_storage.end() ) {
        if ( doSaveState )
            saveState(type); //TODO: сохранение состояния должна быть в другом месте
        removeEntry(found);
    }
}

void ExchangePool::refreshModels() noexcept
{
    for (auto entry : m_storage)
        entry->refresh();
}

void ExchangePool::loadState() noexcept
{
    Exchange::Type type = qobject_cast<ExchangeEntry *>(sender()->parent())->algo()->api()->type();

    auto exch = findByType(type);
    auto api = exch->get()->algo()->api();
    if( !exch->get()->algo()->api()->isLoaded() ) {
        const QString &query = QString(QStringLiteral("SELECT * FROM markets_cache WHERE exchange = '%1'")).arg(static_cast<qint16>(type));

        QSqlQuery &&result = Crud::getInstance()->executeQuery(query);
        IExchangeApiController::MarketsGroup group;
        while (result.next()) {
            const qint32 marketIdIndex = result.record().indexOf(QStringLiteral("market_id"));
            const qint32 minTrendIndex = result.record().indexOf(QStringLiteral("min_trend"));
            const qint32 currentTrendRateIndex = result.record().indexOf(QStringLiteral("cur_trend_rate"));
            const qint32 sellModeIndex = result.record().indexOf(QStringLiteral("sell_mode"));
            const qint32 pauseModeIndex = result.record().indexOf(QStringLiteral("pause_mode"));
            const qint32 percentageProfitIndex = result.record().indexOf(QStringLiteral("percentage_profit"));
            const qint32 isVolumeTradingIndex = result.record().indexOf(QStringLiteral("is_volume_trading"));
            const qint32 timestampIndex = result.record().indexOf(QStringLiteral("timestamp"));
            const qint32 metaConfigIndex = result.record().indexOf(QStringLiteral("meta_config"));
            const qint32 tradeSummIndex = result.record().indexOf(QStringLiteral("trade_summ"));

            const Market::MarketId &marketId = Market::MarketId(result.value(marketIdIndex).toString());
            const qreal &minTrend = result.value(minTrendIndex).toReal();
            const qreal &currentTrendRate = result.value(currentTrendRateIndex).toReal();
            const bool sellMode = result.value(sellModeIndex).toBool();
            const bool pauseMode = result.value(pauseModeIndex).toBool();
            const qreal &percentageProfit = result.value(percentageProfitIndex).toReal();
            const bool isVolumeTrading = result.value(isVolumeTradingIndex).toBool();
            const QDateTime timestamp = result.value(timestampIndex).toDateTime();
            const QJsonObject &metaConfig = QJsonDocument::fromJson(result.value(metaConfigIndex).toByteArray()).object();
            const qreal &tradeSumm = result.value(tradeSummIndex).toReal();

            Market::UserConfig config;
            config.minTrend = minTrend;
            config.currentTrendRate = currentTrendRate;
            config.lastRateUpdateTime = timestamp;
            config.sellMode = sellMode;
            config.pauseMode = pauseMode;
            config.percentageProfit = percentageProfit;
            config.isVolumeTrading = isVolumeTrading;
            config.isTradable = true;
            config.metaConfig = metaConfig;
            config.tradeSumm = tradeSumm;
            group.insert(marketId, config);
        }
        if ( !group.isEmpty() )
            emit addMarket(group);
        api->setLoaded(true);
    }
}
void ExchangePool::saveState(const Exchange::Type type) noexcept
{
    Crud::getInstance()->executeQuery(QString(QStringLiteral("DELETE FROM markets_cache WHERE exchange = '%1'")).arg(static_cast<qint16>(type)));
    auto exch = findByType(type);
    const auto &tradeHistoryFiltered = exch->get()->algo()->api()->marketsStorage()->filterTradingPairs();

    for (const auto &market : tradeHistoryFiltered) {
        const QString &marketId = market->getId().get();
        const qreal &minTrend = market->getMinTrend();
        const qreal &currentTrendRate = market->getCurrentTrendRate();
        const bool sellMode = market->getSellMode();
        const bool pauseMode = market->getPauseMode();
        const qreal &percentageProfit = market->getPercentageProfit();
        const bool isVolumeTrading = market->getIsVolumeTrading();
        const QDateTime &timestamp = market->getLastRateUpdateTime();
        const QJsonDocument doc(market->getMetaConfig());
        const qreal &tradeSumm = market->getTradeSumm();

        const QString &query = QString(QStringLiteral("INSERT INTO markets_cache "
                                                      "(market_id, min_trend, cur_trend_rate, sell_mode, pause_mode, percentage_profit, is_volume_trading, meta_config, exchange, timestamp, trade_summ) "
                                                      "VALUES ('%1', %2, %3, %4, %5, %6, %7, '%8', %9, '%10', %11)"))
                                   .arg(marketId)
                                   .arg(minTrend)
                                   .arg(currentTrendRate)
                                   .arg(sellMode)
                                   .arg(pauseMode)
                                   .arg(percentageProfit)
                                   .arg(isVolumeTrading)
                                   .arg(doc.toJson(QJsonDocument::Compact).constData())
                                   .arg(static_cast<qint16>(type))
                                   .arg(timestamp.toString(Qt::ISODate))
                                   .arg(tradeSumm);
        Crud::getInstance()->executeQuery(query);
    }
}

void ExchangePool::createEntry(const Exchange::Type type) noexcept
{
    auto exch = ITradeAlgorithm::createAlgo(HanukkahAlgo, type); //TODO: dymanic enabled algo's
    connect(this, &ExchangePool::addMarket, exch->api()->controller().get(), &IExchangeApiController::appendMarketsGroup);

    auto analAlgo = AbstractAnalyticsAlgorithm::createAlgo(TrendSearcherAlgo);
    analAlgo->setTradeAlgo(exch);

    const int count = m_storage.count();
    beginInsertRows(QModelIndex(), count, count);

    auto entry = ExchangeEntry::Ptr::create(exch, QList<AbstractAnalyticsAlgorithm::Ptr>() << analAlgo, this);

    connect(analAlgo.get(), &AbstractAnalyticsAlgorithm::updateMarketConfig, qobject_cast<MarketsModel const *>(entry->getMarketsModel()), &MarketsModel::updateMarketConfig);
    connect(qobject_cast<MarketsModel const *>(entry->getMarketsModel()), &MarketsModel::mergeCompeleted, this, &ExchangePool::loadState);
    connect(exch->api()->controller().get(), &IExchangeApiController::removedMarket, qobject_cast<MarketOrdersModel const *>(entry->getMarketOrdersModel()), &MarketOrdersModel::remove);
    connect(exch->api()->controller().get(), &IExchangeApiController::appendedMarket, qobject_cast<MarketOrdersModel const *>(entry->getMarketOrdersModel()), &MarketOrdersModel::insert);
    connect(exch->api()->controller().get(), &IExchangeApiController::removedMarket, qobject_cast<TradeHistoryModel const *>(entry->getTradeHistoryModel()), &TradeHistoryModel::remove);
    connect(exch->api()->controller().get(), &IExchangeApiController::appendedMarket, qobject_cast<TradeHistoryModel const *>(entry->getTradeHistoryModel()), &TradeHistoryModel::insert);
    m_storage << entry;
    endInsertRows();
    exch->setAutoDelete(false);
    analAlgo->setAutoDelete(false);
//    QThreadPool::globalInstance()->start(exch.get());
//    QThreadPool::globalInstance()->start(analAlgo.get());
    exch->run();
    analAlgo->run();
}

void ExchangePool::removeEntry(const ExchangeEntry::Storage::iterator &it) noexcept
{
    it->get()->algo()->stop();
    const qint32 index = static_cast<qint32>(std::distance(m_storage.begin(), it));
    beginRemoveRows(QModelIndex(),index,index);
    m_storage.erase(it);
    endRemoveRows();
}

void ExchangePool::reloadSettings() noexcept
{
    for (auto entry : m_storage)
        entry->reloadSettings();
}

ExchangeEntry::Storage::iterator ExchangePool::findByType(const Exchange::Type type)
{
    auto it = std::find_if(m_storage.begin(), m_storage.end(), [&type](const ExchangeEntry::Ptr &exch){
        return exch->algo()->api()->type() == type;
    });
    return it;
}

ExchangeEntry::ExchangeEntry(ITradeAlgorithm::Ptr algo, const QList<AbstractAnalyticsAlgorithm::Ptr> &anal, QObject *parent) noexcept
    : QObject (parent)
    , m_algo ( algo )
    , m_analyticsPool ( anal )
    , m_marketsModel(new MarketsModel(this))
    , m_marketOrdersModel(new MarketOrdersModel(this))
    , m_openOrdersModel(new OpenOrdersModel(this))
    , m_balancesModel(new BalancesModel(this))
    , m_tradeHistoryModel(new TradeHistoryModel(this))
    , m_marketOrdersProxyModel(new MarketOrdersProxyModel(this))
    , m_buyOrdersProxyModel (new BuyOrdersProxyModel(this))
    , m_sellOrdersProxyModel (new SellOrdersProxyModel(this))
    , m_spreadMarketsProxyModel (new SpreadMarketsProxyModel(this))
    , m_volumeMarketsProxyModel (new VolumeMarketsProxyModel(this))
    , m_tradingPairsProxyModel (new TradingPairsProxyModel(this))
    , m_openOrdersProxyModel (new OpenOrdersProxyModel(this))
    , m_balancesProxyModel (new BalancesProxyModel(this))
    , m_tradeHistoryProxyModel (new TradeHistoryProxyModel(this))
    , m_tradeHistorySelectedPairProxyModel (new TradeHistorySelectedPairsProxyModel(this))
{
    m_marketOrdersProxyModel->setSourceModel(m_marketOrdersModel.get());
    m_buyOrdersProxyModel->setSourceModel(m_marketOrdersProxyModel.get());
    m_sellOrdersProxyModel->setSourceModel(m_marketOrdersProxyModel.get());
    m_spreadMarketsProxyModel->setSourceModel(m_marketsModel.get());
    m_volumeMarketsProxyModel->setSourceModel(m_marketsModel.get());
    m_tradingPairsProxyModel->setSourceModel(m_marketsModel.get());
    m_openOrdersProxyModel->setSourceModel(m_openOrdersModel.get());
    m_balancesProxyModel->setSourceModel(m_balancesModel.get());
    m_tradeHistoryProxyModel->setSourceModel(m_tradeHistoryModel.get());
    m_tradeHistorySelectedPairProxyModel->setSourceModel(m_tradeHistoryModel.get());
    connect(m_algo->api()->controller().get(), &IExchangeApiController::updateMarketConfig, qobject_cast<MarketsModel *>(m_marketsModel.get()), &MarketsModel::updateMarketConfig);
    connect(m_algo.get(), &ITradeAlgorithm::updateMarketConfig, qobject_cast<MarketsModel *>(m_marketsModel.get()), &MarketsModel::updateMarketConfig);
    connect(m_algo->api().get(), &IExchangeAPI::receiveCurrencies, qobject_cast<MarketsModel *>(m_marketsModel.get()), &MarketsModel::mergeCurrencies);
    connect(m_algo->api().get(), &IExchangeAPI::receiveMarketInfo, qobject_cast<MarketsModel *>(m_marketsModel.get()), &MarketsModel::mergeMarketInfo);
    connect(m_algo->api().get(), &IExchangeAPI::receiveMarketData, qobject_cast<MarketsModel *>(m_marketsModel.get()), &MarketsModel::mergeMarketData);
    connect(m_algo->api().get(), &IExchangeAPI::receiveMarketOrders, qobject_cast<MarketOrdersModel *>(m_marketOrdersModel.get()), &MarketOrdersModel::merge);
    connect(m_algo->api()->getEmulator(), &Emulator::loadedMarketOrders, qobject_cast<MarketOrdersModel *>(m_marketOrdersModel.get()), &MarketOrdersModel::merge, Qt::QueuedConnection);
    connect(m_algo->api().get(), &IExchangeAPI::receiveBalance, qobject_cast<BalancesModel *>(m_balancesModel.get()), &BalancesModel::merge);
    connect(m_algo->api()->getEmulator(), &Emulator::loadedBalance, qobject_cast<BalancesModel *>(m_balancesModel.get()), &BalancesModel::merge, Qt::QueuedConnection);
    connect(m_algo->api().get(), &IExchangeAPI::receiveUserOpenOrders, qobject_cast<OpenOrdersModel *>(m_openOrdersModel.get()), &OpenOrdersModel::merge);
    connect(m_algo->api()->getEmulator(), &Emulator::loadedUserOpenOrders, qobject_cast<OpenOrdersModel *>(m_openOrdersModel.get()), &OpenOrdersModel::merge, Qt::QueuedConnection);
    connect(m_algo->api().get(), &IExchangeAPI::receiveUserTradeHistory, qobject_cast<TradeHistoryModel *>(m_tradeHistoryModel.get()), &TradeHistoryModel::merge);
    connect(m_algo->api()->getEmulator(), &Emulator::loadedUserTradeHistory, qobject_cast<TradeHistoryModel *>(m_tradeHistoryModel.get()), &TradeHistoryModel::merge, Qt::QueuedConnection);

    connect(m_algo->api().get(), &IExchangeAPI::receiveCanceledOrders, qobject_cast<OpenOrdersModel *>(m_openOrdersModel.get()), &OpenOrdersModel::remove);
    connect(m_algo->api()->getEmulator(), &Emulator::loadedCanceledOrders, qobject_cast<OpenOrdersModel *>(m_openOrdersModel.get()), &OpenOrdersModel::remove);
}

void ExchangeEntry::refresh() noexcept
{
    qobject_cast<MarketsModel *>(m_marketsModel)->refresh();
    qobject_cast<MarketOrdersModel *>(m_marketOrdersModel)->refresh();
    qobject_cast<OpenOrdersModel *>(m_openOrdersModel)->refresh();
    qobject_cast<BalancesModel *>(m_balancesModel)->refresh();
    qobject_cast<TradeHistoryModel *>(m_tradeHistoryModel)->refresh();
}

void ExchangeEntry::reloadSettings() noexcept
{
    m_algo->reconfigure();
}

const MarketsModel * ExchangeEntry::getMarketsModel() const noexcept
{
    return qobject_cast<MarketsModel *>(m_marketsModel.get());
}

const MarketOrdersModel *ExchangeEntry::getMarketOrdersModel() const noexcept
{
    return qobject_cast<MarketOrdersModel *>(m_marketOrdersModel.get());
}

const TradeHistoryModel *ExchangeEntry::getTradeHistoryModel() const noexcept
{
    return qobject_cast<TradeHistoryModel *>(m_tradeHistoryModel.get());
}
