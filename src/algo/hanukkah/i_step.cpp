#include "i_step.h"
#include <algo/hanukkah.h>
#include <api/i_exchange_api.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <cmath>
#include <crud.h>
#include <Logger.h>

IStep::IStep(const qreal &declinePerc, const qreal &growth, Hanukkah * const parentAlgo, const Market::ConstPtr &market, bool buyBlocked, bool sellBlocked) noexcept
    : m_parentAlgo(parentAlgo)
    , m_market(market)
    , m_declinePerc(declinePerc)
    , m_growth(growth)
    , m_isBuyBlocked(buyBlocked)
    , m_isSellBlocked(sellBlocked) {}

IStep::~IStep() {}

void IStep::setBuyBlocked(bool isBlocked) noexcept
{
    m_isBuyBlocked = isBlocked;
}

void IStep::setSellBlocked(bool isBlocked) noexcept
{
    m_isSellBlocked = isBlocked;
}

IStep::AverageData IStep::calculateAverate(const Order::ConstContainer &container, const QDateTime &cycleTimeStart) noexcept
{
    AverageData ret;
    bool find = false;

    for (const auto &order : container) {
        if ( (order->getType() == Order::Buy)
             && (order->getUpdatedTimeStamp() >= cycleTimeStart)
           ) {
            find = true;
            ret.ordersCount++;
            ret.price += order->getPrice();
            ret.fee += order->getFee();
            ret.baseVolume += order->getBaseVolume();
            ret.volume += order->getCurrencyVolume();
        }

        if ( order->getType() == Order::Sell && find )
            break;
    }
    if ( ret.ordersCount )
        ret.price = std::floor((ret.price / ret.ordersCount) * std::pow(10,8)) * PRECISSION;
    return ret;
}

qreal IStep::calculateStopLoss(const qreal &price, const qreal &oldFee, const qreal &baseVolume, const qreal &fee, const qreal &volume, const qreal &profit)
{
    Q_UNUSED(price)
    const qreal stopLoss = ( ((baseVolume + oldFee) * (1 + profit)) + ((baseVolume + oldFee) * fee) ) / volume;
    return std::floor( stopLoss * std::pow(10,8)) * PRECISSION;
}

void IStep::unlock() noexcept
{
    std::function< void() > callback = std::bind( &Hanukkah::unlockAllSteps, m_parentAlgo, m_market->getId() );
    callback();
}

void IStep::updateMarketMetaConfig(bool isBuyCompleted, const QDateTime &cycleTimeStamp)
{
    Market::UserConfig config = m_market->getUserConfig();
    QJsonObject metaConfig = m_market->getMetaConfig();
    if ( isBuyCompleted && m_isBuyBlocked ) {
        qint32 execSteps = metaConfig.value(QStringLiteral("execSteps")).toInt(0);
        metaConfig.insert(QStringLiteral("execSteps"), ++execSteps);
        const QDateTime &cycleTime = QDateTime::fromString(metaConfig.value(QStringLiteral("cycleTimeStart")).toString(), Qt::ISODate);
        if ( cycleTime.isNull() && !cycleTime.isValid() )
            metaConfig.insert(QStringLiteral("cycleTimeStart"), cycleTimeStamp.toString(Qt::ISODate));
    }

    auto array = metaConfig.value(QStringLiteral("hannukahTable")).toArray();
    for (auto it = array.begin(); it != array.end(); ++it) {
        auto obj = it->toObject();
        const qreal &step = obj.value(QStringLiteral("step")).toDouble();
        const qreal &growth = obj.value(QStringLiteral("growth")).toDouble();
        if ( qFuzzyCompare(step, m_declinePerc) && qFuzzyCompare(growth, m_growth) ) {
            const qint32 indexI = static_cast<qint32>(std::distance(array.begin(), it));
            obj.insert(QStringLiteral("buyStepBlocked"), m_isBuyBlocked);
            obj.insert(QStringLiteral("sellStepBlocked"), m_isSellBlocked);
            array.replace(indexI, obj);
            break;
        }
    }
    metaConfig.insert(QStringLiteral("hannukahTable"), array);
    config.metaConfig = metaConfig;
    QMetaObject::invokeMethod(const_cast<IExchangeApiController *>(m_parentAlgo->api()->controller().get()),
                              "updateMarketConfig",
                              Qt::QueuedConnection,
                              Q_ARG(Market::MarketId, m_market->getId()),
                              Q_ARG(Market::UserConfig, config));

    const QString &marketId = m_market->getId().get();
    qint16 exchangeId = static_cast<qint16>(m_parentAlgo->api()->type());
    QSqlQuery query = Crud::getInstance()->executeQuery(QString(QStringLiteral("SELECT * FROM markets_cache WHERE exchange = %1 AND market_id = '%2'"))
                                                        .arg(exchangeId)
                                                        .arg(marketId));
    const qreal &minTrend = m_market->getMinTrend();
    const qreal &currentTrendRate = m_market->getCurrentTrendRate();
    const bool sellMode = m_market->getSellMode();
    const bool pauseMode = m_market->getPauseMode();
    const qreal &percentageProfit = m_market->getPercentageProfit();
    const bool isVolumeTrading = m_market->getIsVolumeTrading();
    const QDateTime &timestamp = m_market->getLastRateUpdateTime();
    const QJsonDocument doc(metaConfig);
    const qreal &tradeSumm = m_market->getTradeSumm();

    QString cmd;

    if ( query.first() ) {
        cmd = QString(QStringLiteral("UPDATE markets_cache SET "
                                      "min_trend = %1, "
                                      "cur_trend_rate = %2, "
                                      "sell_mode = %3, "
                                      "pause_mode = %4, "
                                      "percentage_profit = %5, "
                                      "is_volume_trading = %6, "
                                      "meta_config = '%7', "
                                      "timestamp = '%8', "
                                      "trade_summ = %9 "
                                      "WHERE exchange = %10 AND market_id = '%11'"));
    } else {
        cmd = QString(QStringLiteral("INSERT INTO markets_cache "
                                      "(min_trend, cur_trend_rate, sell_mode, pause_mode, percentage_profit, is_volume_trading, meta_config, timestamp, trade_summ, exchange, market_id) "
                                      "VALUES (%1, %2, %3, %4, %5, %6, '%7', '%8', %9, %10, '%11')"));


    }
    cmd = cmd.arg(minTrend)
            .arg(currentTrendRate)
            .arg(sellMode)
            .arg(pauseMode)
            .arg(percentageProfit)
            .arg(isVolumeTrading)
            .arg(doc.toJson(QJsonDocument::Compact).constData())
            .arg(timestamp.toString(Qt::ISODate))
            .arg(tradeSumm)
            .arg(exchangeId)
            .arg(marketId);
    LOG_DEBUG(cmd);
    Crud::getInstance()->executeQuery(cmd);
}
