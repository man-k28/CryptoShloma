#include "balances_storage.h"
#include <Logger.h>
#include <common/constants.h>
#include <api/i_exchange_api.h>
#include "markets_storage.h"
#include <QWriteLocker>

BalancesStorage::BalancesStorage(IExchangeAPI *parent) noexcept
    : QObject()
    , m_api(parent)
{
}

Balance::ConstPtr BalancesStorage::find(const Currency::ConstPtr &currency) const noexcept
{
    LOG_TRACE_TIME();
    auto found = std::find_if(m_Data.begin(), m_Data.end(), [&currency](const Balance::Ptr &item)
    {
        return item->getCurrency()->getId() == currency->getId();
    });
    if ( found == m_Data.end() ) {
        LOG_TRACE(QString(QStringLiteral("Balance %1 not found")).arg(currency->getId().get()));
        return {};
    }
    return *found;
}

Balance::ConstPtr BalancesStorage::find(const Currency::CurrencyId &id) const noexcept
{
    LOG_TRACE_TIME();
    auto found = std::find_if(m_Data.begin(), m_Data.end(), [&id](const Balance::Ptr &item)
                              {
                                  return item->getCurrency()->getId() == id;
                              });
    if ( found == m_Data.end() ) {
        LOG_TRACE(QString(QStringLiteral("Balance %1 not found")).arg(id.get()));
        return {};
    }
    return *found;
}

Balance::ConstPtr BalancesStorage::btcBalance() const noexcept
{
    LOG_TRACE_TIME();
    //TODO: можно оптимизировать, найти один раз и запомнить указатель
    auto found = std::find_if(m_Data.begin(), m_Data.end(), [](const Balance::Ptr &item)
    {
        return item->getCurrency()->getId().get() == CONST_BTC;
    });
    if ( found == m_Data.end() ) {
        LOG_WARNING(QStringLiteral("Balance BTC not found"));
        return {};
    }
    return *found;
}

void BalancesStorage::merge(const QList<exchange::BalanceEntry> &data) noexcept
{
    LOG_TRACE_TIME();
    QReadLocker locked(&lock);
//    for (auto i = m_Data.begin(); i != m_Data.end();) {
//        auto found = std::find_if(data.begin(), data.end(),
//            [ &i ]( const exchange::BalanceEntry &entry ) noexcept {
//                return entry.currencyId == (*i)->getCurrency()->getId();
//            });

//        if ( found == data.cend() )
//            i = m_Data.erase(i);
//        else
//            ++i;
//    }

    for (const auto &item : data) {
        auto found = std::find_if(m_Data.begin(), m_Data.end(),
            [ &item ]( const Balance::ConstPtr &entry ) noexcept {
                return entry->getCurrency()->getId() == item.currencyId;
            });
        if ( found != m_Data.end() ) {
            if ( item.params.timestamp >= found->get()->getTimestamp() )
                found->get()->setConfig(item.params);
        } else {
            auto balance = Balance::Ptr::create(item.params);

            const auto &currency = m_api->marketsStorage()->findCurrencyById(item.currencyId);

            if ( currency.isNull())
                continue;
            balance->setCurrency(currency);
            m_Data.append(balance);
        }
    }
}
