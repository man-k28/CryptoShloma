#ifndef BALANCES_STORAGE_H
#define BALANCES_STORAGE_H

#include <api/i_exchange_api_entry.h>
#include <QReadWriteLock>
class IExchangeAPI;

class BalancesStorage final : public QObject
{
    Q_OBJECT
public:
    explicit BalancesStorage(IExchangeAPI *parent = nullptr) noexcept;
    explicit BalancesStorage(const BalancesStorage &o) noexcept = delete;
    BalancesStorage& operator=(const BalancesStorage &o) noexcept = delete ;
public:
    Balance::ConstPtr find(const Currency::ConstPtr &currency) const noexcept;
    Balance::ConstPtr find(const Currency::CurrencyId &id) const noexcept;
    Balance::ConstPtr btcBalance() const noexcept; //TODO: deprecated
public slots:
    void merge(const QList<exchange::BalanceEntry> &data) noexcept;
private:
    QList<Balance::Ptr> m_Data{};
    IExchangeAPI * const m_api;
    QReadWriteLock lock{};
};

#endif // BALANCES_STORAGE_H
