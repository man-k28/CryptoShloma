#ifndef MARKETS_STORAGE_H
#define MARKETS_STORAGE_H

#include <api/i_exchange_api_entry.h>
#include <QReadWriteLock>
class MarketsStorage final : public QObject
{
    Q_OBJECT
public:
    explicit MarketsStorage(QObject *parent = nullptr) noexcept;
public:
    const Currency::ConstPtr findCurrencyById(const Currency::CurrencyId &id) const noexcept;
    const Currency::ConstPtr findCurrencyBySymbol(const QString &symbol) const noexcept;
    const Market::ConstPtr findMarketById(const Market::MarketId &id) const noexcept;
    const Market::ConstPtr findMarketByCurrencyId(const Currency::CurrencyId &id) const noexcept;
    const Market::ConstPtr findMarketBySymbol(const QString &symbol) const noexcept;
    const Market::ConstPtr findMarketByLabel(const QString &label) const noexcept;
    const Market::MarketId findMarketIdBySymbol(const QString &symbol) const noexcept;

    const Market::Container filterTradingPairs() const noexcept;
//    quint32 getCurrencysSequential() const noexcept;
//    quint32 getMarketsSequential() const noexcept;
public slots:
    void mergeCurrencies(const QList<Currency::Config> &data) noexcept;
    void mergeMarketInfo(const QList<exchange::MarketEntry> &data) noexcept;
    void mergeMarketData(const QList<exchange::MarketEntry> &data) noexcept;
    void updateMarketUserConfig(const Market::MarketId &marketId, const Market::UserConfig &config) noexcept;
signals:
    void removeTradePairIdFromTrading(const Market::MarketId &id);
private slots:
    void cancelTradingMarket(const Market::MarketId &id) noexcept;
private:
    Market::Container m_Data{};
    Currency::Container m_Currencys{};
    QReadWriteLock lock{};
};

#endif // MARKETS_STORAGE_H
