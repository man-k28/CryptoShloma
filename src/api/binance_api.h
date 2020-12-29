#ifndef BINANCEAPI_H
#define BINANCEAPI_H

#include <api/i_exchange_api.h>
#include <Logger.h>

class WebClient;
class WebSocketClient;
class BinanceAPIPrivate;
class QTimer;

class BinanceAPI final: public IExchangeAPI
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(BinanceAPI)
public:

    explicit BinanceAPI() noexcept;
    explicit BinanceAPI(const BinanceAPI &o) noexcept = delete;
    BinanceAPI& operator=(const BinanceAPI &o) noexcept = delete;
    ~BinanceAPI() noexcept override;
    Exchange::Type type() const noexcept override { return Exchange::Type::Binance; }

    void start() noexcept override;
    void stop() noexcept override;

    QString cancelOrderTypeToString(const CancellationOrderType type) const noexcept override;

    Market::MarketStatus marketStatusToEnum(const QString &status) const noexcept override;
    QString marketStatusToString(const Market::MarketStatus status) const noexcept override;

    Currency::CurrencyStatus currencyStatusToEnum(const QString &status) const noexcept override;
    QString currencyStatusToString(const Currency::CurrencyStatus status) const noexcept override;

    Currency::CurrencyStatusListing currencyListingStatusToEnum(const QString &status) const noexcept override;
    QString currencyListingStatusToString(const Currency::CurrencyStatusListing status) const noexcept override;

    Balance::BalanceStatus balanceStatusToEnum(const QString &status) const noexcept override;
    QString balanceStatusToString(const Balance::BalanceStatus status) const noexcept override;

    Order::OrderType orderTypeToEnum(const QString &type) const noexcept override;
    QString orderTypeToString(const Order::OrderType type) const noexcept override;

    /*!
     * \brief Public API
     */
    void serverTime() noexcept override;
    void listCurrencies() const override;
    void listMarketInfo() const override;
    void listMarketData(const Market::MarketId &baseMarket = Market::MarketId(CONST_BTC), //WARNING: пока только BTC
                        const quint16 hours = 24) const override;
    void listMarketOrders(const Market::MarketId &id,
                          const quint16 count = CONST_ORDERS_COUNT) const override;
    void removeListMarketOrders(const Market::MarketId &marketId) override;
    void listMarketOrderGroups(const QList<Market::MarketId> &markets,
                               const quint16 count = CONST_ORDERS_COUNT) const override;

    /*!
     * \brief Private API
     */

    void userAuth() const override;
    void userBalance(const Currency::CurrencyId &currencyId = Currency::CurrencyId()) const override;
    void userOpenOrders(const Market::MarketId &market = Market::MarketId(),
                        const quint16 count = 50) const override;

    void submitOrder(const Market::MarketId &id,
                     const Order::OrderType side,
                     const qreal &price,
                     const qreal &baseVolume,
                     const qreal &currencyVolume = 0.) override; //FIXME: сделать правильно выставление по базовой валюте, либо по торгуемой, а то можно проебаться

    void cancelOrder(const CancellationOrderType type,
                     const Order::OrderId &orderId = Order::OrderId(0),
                     const Market::MarketId &id = Market::MarketId()) override;

    void userTradeHistory(const Market::MarketId &market = Market::MarketId(),
                          const quint16 count = 500) const override;

    void withdraw(const Currency::CurrencyId &currencyId,
                  const qreal &volume,
                  const QString &address) const override;
protected:
    QScopedPointer<BinanceAPIPrivate> d_ptr;
    explicit BinanceAPI(BinanceAPIPrivate &dd, QObject * parent = nullptr);
private slots:
    void timerEvent(QTimerEvent *e) noexcept override final;
    void httpError(QNetworkReply::NetworkError code);
    void logOut();
    void deleteToken();

    void processExchangeInfo();
    void processMarketData();
    void processBalance();
    void processUserTradeHistory();
    void processUserOpenOrders();
    void processCancelOrders();
    void processNewOrders();

    void processStreams(const QString &message);
    void processUserStreams(const QString &message);
    void processUserAuth();
private:
    auto processingReply(QNetworkReply *reply) noexcept -> QJsonDocument;
    void processAllMarketTicketStream(const QJsonObject &root) noexcept;
    void processPartialBookDepthStream(const QJsonObject &root) noexcept;

    void processOutboundAccountInfo(const QJsonObject &root) noexcept;
    void processExecutionReportStream(const QJsonObject &root) noexcept;
    QNetworkRequest prepareMbxRequest(const QString &apiKey,
                                      const QString &secretKey,
                                      const QUrl &url,
                                      QByteArray &params,
                                      bool requestBody = false,
                                      bool needAppenParams = true) const;
private:
    WebClient * const m_webClient = nullptr;
    WebSocketClient * const m_webSocketMarketsClient = nullptr;
    WebSocketClient * const m_webSocketUserDataClient = nullptr;
    qint64 m_serverTimeOffset = 0;
    qint32 m_userWSPutKeyTimerId = 0;
    qint32 m_userWSPingTimerId = 0;
    QString m_userToken = {};
    QTimer *m_requestBreakTimer = nullptr;
    bool m_requestsIsBlock = false;
};

#endif // BINANCEAPI_H
