#include "hanukkah.h"
#include "hanukkah/step.h"
#include "hanukkah/zero_step.h"
#include <QJsonObject>
#include <QTimerEvent>
#include <Logger.h>
#include <api/i_exchange_api.h>
#include <storage/markets_storage.h>
#include <crud.h>

//TODO: отсутствует очистка сессии при удаление пары с торгов
Hanukkah::Hanukkah(QObject *parent) noexcept
    : ITradeAlgorithm(parent)
{
}

Hanukkah::~Hanukkah() noexcept
{
    this->stop();
}

void Hanukkah::unlockAllSteps(const Market::MarketId &id) noexcept
{
    auto &session = m_sessionContainer[id];
    for ( auto it = session.container.begin(); it != session.container.end(); ++it) {
        it->get()->setBuyBlocked(false);
        it->get()->setSellBlocked(false);
    }

    Market::UserConfig config = session.market()->getUserConfig();
    QJsonObject metaConfig = session.market()->getMetaConfig();
    metaConfig.insert(QStringLiteral("execSteps"), 0);
    auto array = metaConfig.value(QStringLiteral("hannukahTable")).toArray();
    for (auto it = array.begin(); it != array.end(); ++it) {
        auto obj = it->toObject();
        const qint32 indexI = static_cast<qint32>(std::distance(array.begin(), it));
        obj.insert(QStringLiteral("buyStepBlocked"), false);
        obj.insert(QStringLiteral("sellStepBlocked"), false);
        array.replace(indexI, obj);
    }
    metaConfig.insert(QStringLiteral("hannukahTable"), array);
    metaConfig.insert(QStringLiteral("cycleTimeStart"),"");
    config.metaConfig = metaConfig;
    QMetaObject::invokeMethod(const_cast<IExchangeApiController *>(api()->controller().get()),
                              "updateMarketConfig",
                              Qt::QueuedConnection,
                              Q_ARG(Market::MarketId, session.market()->getId()),
                              Q_ARG(Market::UserConfig, config));
    Crud::getInstance()->executeQuery(QString("DELETE FROM markets_cache WHERE exchange = %1 AND market_id = '%2'")
                                      .arg(static_cast<qint16>(api()->type()))
                                      .arg(id.get()));
}

bool Hanukkah::start() noexcept
{
    if ( m_timerId == 0)
        m_timerId = startTimer(1000);
    return true;
}

bool Hanukkah::stop() noexcept
{
    if ( m_timerId ) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
    emit finished();
    return true;
}

bool Hanukkah::restart() noexcept
{
    return true;
}

void Hanukkah::reconfigure() noexcept
{

}

void Hanukkah::timerEvent(QTimerEvent *event) noexcept
{
    Q_UNUSED(event)

    if ( event->timerId() == m_timerId ) {
        execLogic();
    }
}

void Hanukkah::removeMarket(const Market::MarketId &id) noexcept
{
    m_sessionContainer.remove(id);
}

void Hanukkah::execLogic() noexcept
{
    const auto &markets = api()->marketsStorage()->filterTradingPairs();

    for (const auto &market : markets) {
        const auto &metaConfig = market->getMetaConfig();
        const auto &array = metaConfig.value(QStringLiteral("hannukahTable")).toArray();
        const qint32 execSteps = metaConfig.value(QStringLiteral("execSteps")).toInt();
        const qint32 maxSteps = metaConfig.value(QStringLiteral("maxSteps")).toInt();
        SessionContainer::iterator itSession = m_sessionContainer.find(market->getId());

        if ( itSession == m_sessionContainer.end() )
            itSession = m_sessionContainer.insert(market->getId(), StepGenerator(array, this, market));
        else
            itSession->update(array);

        for ( const auto &item : itSession.value().container ) {
            if ( !item->isBuyBlocked() && ( execSteps <= maxSteps ) ) {
                item->executeBuyLogic();
                break;
            }
            if (!item->isSellBlocked() ) {
                item->executeSellLogic();
                break;
            }
        }
    }
}

Hanukkah::StepGenerator::StepGenerator(const QJsonArray &data, Hanukkah * const parentAlgo, const Market::ConstPtr &market) noexcept
    : metaConfig(data)
    , m_parentAlgo(parentAlgo)
    , m_market(market)
{
    setup();
}

Hanukkah::StepGenerator::StepGenerator(const Hanukkah::StepGenerator &o) noexcept
    : metaConfig(o.metaConfig)
    , container(o.container)
    , m_parentAlgo(o.m_parentAlgo)
    , m_market(o.m_market)
{
}

Hanukkah::StepGenerator &Hanukkah::StepGenerator::operator=(const Hanukkah::StepGenerator &o) noexcept
{
    m_market = o.m_market;
    container = o.container;
    m_parentAlgo = o.m_parentAlgo;
    metaConfig = o.metaConfig;
    return *this;
}

bool Hanukkah::StepGenerator::update(const QJsonArray &data) noexcept
{
    if ( metaConfig != data ) {
        metaConfig = data;
        if ( container.count() ) {
            container.clear();
            container.squeeze();
        }
        setup();
        return true;
    }
    return false;
}

void Hanukkah::StepGenerator::setup() noexcept
{
    for (auto it = metaConfig.begin(); it != metaConfig.end(); ++it) {
        const auto &obj = it->toObject();
        const qreal &step = obj.value(QStringLiteral("step")).toDouble();
        const qreal &growth = obj.value(QStringLiteral("growth")).toDouble();
        const bool buyStepBlocked = obj.value(QStringLiteral("buyStepBlocked")).toBool();
        const bool sellStepBlocked = obj.value(QStringLiteral("sellStepBlocked")).toBool();
        //WARNING: здесь аккуратно, параметры шага 0 должны быть 0. переделать
        if ( it == metaConfig.begin() )
            container.insert_back(QSharedPointer<ZeroStep>::create(step, growth, m_parentAlgo, m_market, buyStepBlocked, sellStepBlocked));
        else
            container.insert_back(QSharedPointer<Step>::create(step, growth, m_parentAlgo, m_market, buyStepBlocked, sellStepBlocked));
    }
}

Market::ConstPtr Hanukkah::StepGenerator::market() const
{
    return m_market;
}
