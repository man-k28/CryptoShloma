#include "markets_storage.h"
#include <Logger.h>

MarketsStorage::MarketsStorage(QObject *parent) noexcept
    : QObject()
{
    Q_UNUSED(parent)
}

const Currency::ConstPtr MarketsStorage::findCurrencyById(const Currency::CurrencyId &id) const noexcept
{
    auto found = std::find_if(m_Currencys.begin(), m_Currencys.end(), [&id](const Currency::Ptr &item)
    {
        return item->getId() == id;
    });
    if ( found == m_Currencys.end() )
        return {};
    return *found;
}

const Currency::ConstPtr MarketsStorage::findCurrencyBySymbol(const QString &symbol) const noexcept
{
    auto found = std::find_if(m_Currencys.begin(), m_Currencys.end(), [&symbol](const Currency::Ptr &item)
    {
        return !item->getId().get().compare(symbol, Qt::CaseInsensitive);
    });
    if ( found == m_Currencys.end() )
        return {};
    return *found;
}

const Market::ConstPtr MarketsStorage::findMarketById(const Market::MarketId &id) const noexcept
{
    auto found = std::find_if(m_Data.begin(), m_Data.end(), [&id](const Market::Ptr &item)
    {
        return item->getId() == id;
    });
    if ( found == m_Data.end() )
        return {};
    return *found;
}

const Market::ConstPtr MarketsStorage::findMarketByCurrencyId(const Currency::CurrencyId &id) const noexcept
{
    auto found = std::find_if(m_Data.begin(), m_Data.end(), [&id](const Market::Ptr &item)
    {
        return item->getCurrency()->getId() == id;
    });
    if ( found == m_Data.end() )
        return {};
    return *found;
}

const Market::ConstPtr MarketsStorage::findMarketBySymbol(const QString &symbol) const noexcept
{
    auto found = std::find_if(m_Data.begin(), m_Data.end(), [&symbol](const Market::Ptr &item)
    {
        return !item->getCurrency()->getId().get().compare(symbol, Qt::CaseInsensitive);
    });
    if ( found == m_Data.end() )
        return {};
    return *found;
}

const Market::ConstPtr MarketsStorage::findMarketByLabel(const QString &label) const noexcept
{
    auto found = std::find_if(m_Data.begin(), m_Data.end(), [&label](const Market::Ptr &item)
    {
        return !item->getId().get().compare(label, Qt::CaseInsensitive);
    });
    if ( found == m_Data.end() )
        return {};
    return *found;
}

const Market::MarketId MarketsStorage::findMarketIdBySymbol(const QString &symbol) const noexcept
{
    auto found = std::find_if(m_Data.begin(), m_Data.end(), [&symbol](const Market::Ptr &item)
    {
        return !item->getCurrency()->getId().get().compare(symbol, Qt::CaseInsensitive);
    });
    if ( found == m_Data.end())
        return Market::MarketId();
    else
        return found->get()->getId();
}

const Market::Container MarketsStorage::filterTradingPairs() const noexcept
{
    Market::Container filtered;

    std::copy_if(m_Data.begin(), m_Data.end(),std::back_inserter(filtered), [](const Market::Ptr &market){
        return market->getIsTradable();
    });
    return filtered;
}

//quint32 MarketsStorage::getCurrencysSequential() const noexcept
//{
////#ifdef CH_USE_THREADS
////    QReadLocker locker(&m_DataLock);
////#endif
//    return static_cast<quint32>(m_Currencys.count() + 1);
//}

//quint32 MarketsStorage::getMarketsSequential() const noexcept
//{
////#ifdef CH_USE_THREADS
////    QReadLocker locker(&m_DataLock);
////#endif
//    return static_cast<quint32>(m_Data.count() + 1);
//}

void MarketsStorage::mergeMarketData(const QList<exchange::MarketEntry> &data) noexcept
{
    LOG_TRACE_TIME();
    QReadLocker locked(&lock);
    for (const auto &item : data) {
        auto found = std::find_if(m_Data.begin(), m_Data.end(),
            [ & ]( const decltype( m_Data )::value_type &market ) noexcept
            {
                return market->getId() == item.params.id;
            });
        if ( found != m_Data.end() )
            found->get()->setConfig(item.params);
    }
}

void MarketsStorage::updateMarketUserConfig(const Market::MarketId &marketId, const Market::UserConfig &config) noexcept
{
    QReadLocker locked(&lock);
    for (auto market : m_Data) {
        if ( market->getId() == marketId ) {
            market->setUserConfig(config);
            break;
        }
    }
}

void MarketsStorage::cancelTradingMarket(const Market::MarketId &id) noexcept
{
    QReadLocker locked(&lock);
    for (auto market : m_Data) {
        if ( market->getId() == id ) {
            market->setIsTradable(false);
            break;
        }
    }
}

void MarketsStorage::mergeMarketInfo(const QList<exchange::MarketEntry> &data) noexcept
{
    LOG_TRACE_TIME();
    QReadLocker locked(&lock);
    for (auto item = m_Data.begin(); item != m_Data.end();) {
        auto found = std::find_if(data.cbegin(), data.cend(),
                                  [ &item ]( const exchange::MarketEntry &market ) noexcept
                                  {
                                      return market.params.id == item->get()->getId();
                                  });
        if ( found == data.cend() )
            item = m_Data.erase(item);
        else
            ++item;
    }

    for (const auto &item : data) {
        auto found = std::find_if(m_Data.begin(), m_Data.end(),
                                  [ &item ]( const decltype( m_Data )::value_type &market ) noexcept
                                  {
                                      return market->getId() == item.params.id;
                                  });
        if ( found != m_Data.end() )
            found->get()->setConfig(item.params);
        else {
            auto market = Market::Ptr::create(item.params);

            const auto &currency = findCurrencyById(item.tradeCurrencyId);
            const auto &baseCurrency = findCurrencyById(item.baseCurrencyId);

            if ( currency.isNull() || baseCurrency.isNull() )
                continue;
            market->setCurrency(currency);
            market->setBaseCurrency(baseCurrency);
            m_Data.append(market);
        }
    }
}

void MarketsStorage::mergeCurrencies(const QList<Currency::Config> &data) noexcept
{
    LOG_TRACE_TIME();
    QReadLocker locked(&lock);
    for (auto item = m_Currencys.begin(); item != m_Currencys.end();) {
        auto found = std::find_if(data.cbegin(), data.cend(),
                                  [ &item ]( const Currency::Config &currency ) noexcept
                                  {
                                      return currency.id == item->get()->getId();
                                  });
        if ( found == data.cend() )
            item = m_Currencys.erase(item);
        else
            ++item;
    }

    for (const auto &item : data) {
        auto found = std::find_if(m_Currencys.begin(), m_Currencys.end(),
                                  [ &item ]( const decltype( m_Currencys )::value_type &currency ) noexcept
                                  {
                                      return currency->getId() == item.id;
                                  });
        if ( found != m_Currencys.end() )
            found->get()->setConfig(item);
        else
            m_Currencys.append(Currency::Ptr::create(item));
    }
}
