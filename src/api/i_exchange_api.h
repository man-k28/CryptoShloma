#ifndef IEXCHANGEAPI_H
#define IEXCHANGEAPI_H
#include <QRunnable>
#include <QNetworkReply>
#include <common/constants.h>
#include <common/enums.h>
#include "i_exchange_api_controller.h"
#include "i_exchange_api_entry.h"

class MarketsStorage;
class MarketOrdersStorage;
class OpenOrdersStorage;
class TradeHistoryStorage;
class BalancesStorage;
class TemporaryOrdersTable;
class Emulator;

class IExchangeAPI : public QObject, public QRunnable
{
    Q_OBJECT
    Q_PROPERTY(Exchange::Type type READ type CONSTANT)
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY isOnlineChanged)
    Q_PROPERTY(IExchangeApiController *controller READ getController CONSTANT)
    Q_PROPERTY(Emulator *emulator READ getEmulator CONSTANT)
public:
    enum CancellationOrderType {
        All,
        OrderId,
        MarketId
    };
    Q_ENUM(CancellationOrderType)
    using Ptr = QSharedPointer<IExchangeAPI>;
public:
    explicit IExchangeAPI(QObject *parent = nullptr) noexcept;
    explicit IExchangeAPI(const IExchangeAPI &o) noexcept = delete;
    IExchangeAPI& operator=(const IExchangeAPI &o) noexcept = delete;
    virtual ~IExchangeAPI() override;

    static Ptr createExchangeApi(const Exchange::Type type) noexcept;
    Emulator *getEmulator() noexcept; //FIXME: сделать const

    virtual Exchange::Type type() const noexcept = 0;
    inline bool isOnline() const noexcept
    {
        return m_isOnline;
    }

    virtual void start() noexcept = 0;
    virtual void stop() noexcept = 0;
private:
    void run() noexcept override final;
public:
    virtual QString cancelOrderTypeToString(const CancellationOrderType type) const noexcept = 0;

    virtual Market::MarketStatus marketStatusToEnum(const QString &status) const noexcept = 0;
    virtual QString marketStatusToString(const Market::MarketStatus status) const noexcept = 0;

    virtual Currency::CurrencyStatus currencyStatusToEnum(const QString &status) const noexcept = 0;
    virtual QString currencyStatusToString(const Currency::CurrencyStatus status) const noexcept = 0;

    virtual Currency::CurrencyStatusListing currencyListingStatusToEnum(const QString &status) const noexcept = 0;
    virtual QString currencyListingStatusToString(const Currency::CurrencyStatusListing status) const noexcept = 0;

    virtual Balance::BalanceStatus balanceStatusToEnum(const QString &status) const noexcept = 0;
    virtual QString balanceStatusToString(const Balance::BalanceStatus status) const noexcept = 0;

    virtual Order::OrderType orderTypeToEnum(const QString &type) const noexcept = 0;
    virtual QString orderTypeToString(const Order::OrderType type) const noexcept = 0;
public:
    virtual void serverTime() = 0;
    virtual void listCurrencies() const = 0;
    virtual void listMarketInfo() const = 0;
    virtual void listMarketData(const Market::MarketId &baseMarket = Market::MarketId(CONST_BTC),
                                const quint16 hours = 24) const = 0;
    virtual void listMarketOrders(const Market::MarketId &marketId,
                                  const quint16 count = CONST_ORDERS_COUNT) const = 0;
    virtual void removeListMarketOrders(const Market::MarketId &marketId) = 0;
    virtual void listMarketOrderGroups(const QList<Market::MarketId> &markets,
                                       const quint16 count = CONST_ORDERS_COUNT) const = 0;

    virtual void userAuth() const = 0;
    virtual void userBalance(const Currency::CurrencyId &currencyId = Currency::CurrencyId()) const = 0;
    virtual void userOpenOrders(const Market::MarketId &market = Market::MarketId(),
                                const quint16 count = 50) const = 0;

    virtual void submitOrder(const Market::MarketId &marketId,
                             const Order::OrderType side,
                             const qreal &price,
                             const qreal &baseVolume,
                             const qreal &currencyVolume = 0.) = 0;

    virtual void cancelOrder(const CancellationOrderType type,
                             const Order::OrderId &orderId = Order::OrderId(0),
                             const Market::MarketId &marketId = Market::MarketId()) = 0;

    virtual void userTradeHistory(const Market::MarketId &marketId = Market::MarketId(),
                              const quint16 count = 500) const = 0;

    virtual void withdraw(const Currency::CurrencyId &currencyId,
                          const qreal &volume,
                          const QString &address) const = 0;
public:
    inline MarketOrdersStorage * marketOrdersStorage() const noexcept
    {
        return m_marketOrdersStorage;
    }

    inline MarketsStorage const * marketsStorage() const noexcept
    {
        return m_marketsStorage;
    }

    inline OpenOrdersStorage const * openOrdersStorage() const noexcept
    {
        return m_openOrdersStorage;
    }

    inline BalancesStorage const * balancesStorage() const noexcept
    {
        return m_balancesStorage;
    }

    inline TradeHistoryStorage * tradeHistoryStorage() const noexcept
    {
        return m_tradeHistoryStorage;
    }

    inline bool isLoaded() const noexcept
    {
        return m_IsLoaded;
    }
    void setLoaded(bool value) noexcept
    {
        m_IsLoaded = value;
    }

    inline TemporaryOrdersTable * tempOrders() const noexcept
    {
        return m_tempOrdersTable;
    }

    const IExchangeApiController::ConstPtr controller() const noexcept;

    TemporaryOrdersTable * tempOrdersTable() const noexcept;

protected:
    virtual void calculateSpreadForMarket(Market::Config &config) const noexcept;
protected slots:
    void setOffline() noexcept;
    void setOnline() noexcept;
signals:
    void receiveCurrencies(const QList<Currency::Config> &list);
    void receiveMarketInfo(const QList<exchange::MarketEntry> &list);
    void receiveMarketData(const QList<exchange::MarketEntry> &list);
    void receiveMarketOrders(const QList<exchange::OrderEntry> &list);
    void receiveUserOpenOrders(const QList<exchange::OrderEntry> &list);
    void receiveBalance(const QList<exchange::BalanceEntry> &list);
    void receiveCanceledOrders(const QList<Order::OrderId> &orders);
    void receiveUserTradeHistory(const QList<exchange::OrderEntry> &list);
signals:
    void isOnlineChanged();

    void emulSubmitOrder(const Market::MarketId &marketId,
                         const Order::OrderId &orderId,
                         const Order::OrderType side,
                         const qreal &price,
                         const qreal &baseVolume,
                         const qreal &currencyVolume = 0.);
    void emulCanceLorder(const Order::OrderId &orderId = Order::OrderId(0),
                         const Market::MarketId &marketId = Market::MarketId());
protected:
    bool                        m_isOnline{false};
    IExchangeApiController::Ptr m_controller{};
signals:
    void disconnectedPublicApi();
    void disconnectedPrivateApi();
private:
    IExchangeApiController * getController() const noexcept;
private:
//    QThread                    *m_ModelsThread = nullptr;
    MarketsStorage             *m_marketsStorage;
    MarketOrdersStorage        *m_marketOrdersStorage;
    OpenOrdersStorage          *m_openOrdersStorage;
    BalancesStorage            *m_balancesStorage;
    TradeHistoryStorage        *m_tradeHistoryStorage;
    bool                        m_IsLoaded{false};
    TemporaryOrdersTable       * const m_tempOrdersTable = nullptr; //FIXME: доделать
    Emulator                   * const m_emulator = nullptr;
};

#endif // IEXCHANGEAPI_H
