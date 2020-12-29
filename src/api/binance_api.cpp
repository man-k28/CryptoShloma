#include "binance_api.h"
#include "binance_api_p.h"
#include <QEventLoop>
#include <QTimer>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QMessageAuthenticationCode>
#include <common/utils.h>
#include "binance_api_controller.h"
#include <network/http_client.h>
#include <network/websocket_client.h>
#include <common/settings_manager.h>
#include <crud.h>
#include "temporary_orders_table.h"
#include <storage/markets_storage.h>
#include <random>
#include <RollingFileAppender.h>
#include <QDir>

BinanceAPI::BinanceAPI() noexcept
    : IExchangeAPI()
    , d_ptr( new BinanceAPIPrivate(this) )
    , m_webClient( new WebClient(this) )
    , m_webSocketMarketsClient( new WebSocketClient(this) )
    , m_webSocketUserDataClient( new WebSocketClient(this) )
    , m_requestBreakTimer( new QTimer(this) )
{
    Q_D(BinanceAPI);
    d->q_ptr = this;
    //TODO: подумать как перенести в обязательное создание в базовом классе
    m_controller = IExchangeApiController::Ptr(new BinanceAPIController(this));
    m_webSocketMarketsClient->setBaseTemplate(d->EndPoints().value(d->URLWebSockets) + QStringLiteral("stream?streams="));
    m_webSocketUserDataClient->setBaseTemplate(d->EndPoints().value(d->URLWebSockets) + QStringLiteral("ws/"));

    connect(m_webSocketMarketsClient->get(), &QWebSocket::textMessageReceived, this, &BinanceAPI::processStreams);
    connect(m_webSocketMarketsClient->get(), &QWebSocket::disconnected, [this](){
        setOffline();
        emit disconnectedPublicApi();
    });
    connect(m_webSocketMarketsClient->get(), &QWebSocket::connected, this, &BinanceAPI::setOnline);
    //FIXME: немного костыль, надо бы привязаться глобально к объекту
    connect(m_webSocketMarketsClient->session(), &QNetworkSession::opened, this, &IExchangeAPI::disconnectedPublicApi);
    connect(m_webSocketMarketsClient->session(), &QNetworkSession::opened, this, &IExchangeAPI::disconnectedPrivateApi);

    connect(m_webSocketUserDataClient->get(), &QWebSocket::textMessageReceived, this, &BinanceAPI::processUserStreams);
    connect(m_webSocketUserDataClient->get(), &QWebSocket::disconnected, [this](){
        setOffline();
        emit disconnectedPrivateApi();
        //FIXME: logout
        //        logOut();
    });
    connect(m_webSocketUserDataClient->get(), &QWebSocket::connected, this, &BinanceAPI::setOnline);

    m_requestBreakTimer->setSingleShot(true);
    connect(m_requestBreakTimer, &QTimer::timeout, [this] () {
        m_requestsIsBlock = false;
        emit disconnectedPublicApi();
        emit disconnectedPrivateApi();
    });

#ifndef QT_NO_DEBUG
    const QString logDirName{qApp->applicationDirPath() + QStringLiteral("/logs/binance")};
#else
    const QString logDirName{QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QString(QStringLiteral("/.%1/logs/binance")).arg(GLOBAL_APP_NAME)};
#endif
    QDir logDir{logDirName};
    if ( !logDir.exists() )
        logDir.mkpath(logDirName);

    const QString logFormat{QStringLiteral("[%{time}{dd-MM-yyyy HH:mm:ss}] [%{type}] <%{function}:%{line}> %{message}\n")};
    const QString &fileFormat = QString("%1/%2.log")
                                    .arg(logDirName)
                                    .arg(qApp->applicationName());
    RollingFileAppender *rollingFileAppender = new RollingFileAppender(fileFormat);
    rollingFileAppender->setFormat(logFormat);
    rollingFileAppender->setDatePattern(RollingFileAppender::DailyRollover);
    cuteLogger->registerCategoryAppender("BinanceAPI.net", rollingFileAppender);
}

BinanceAPI::BinanceAPI(BinanceAPIPrivate &dd, QObject *parent)
    : IExchangeAPI(parent)
    , d_ptr( &dd )
{
    Q_D(BinanceAPI);
    d->q_ptr = this;
}

void BinanceAPI::timerEvent(QTimerEvent *e) noexcept
{
    Q_D(BinanceAPI);

    if ( e->timerId() == m_userWSPutKeyTimerId && !m_userToken.isEmpty() && !m_requestsIsBlock ) {
        QByteArray params;
        params.append(QString("?listenKey=%1").arg(m_userToken));
        auto reply = m_webClient->put(prepareMbxRequest(SettingsManager::get(SettingsManager::BinanceAPIKey, SettingsManager::Binance).toString(),
                                                                               SettingsManager::get(SettingsManager::BinanceSecretKey, SettingsManager::Binance).toString(),
                                                                               QUrl(d->EndPoints().value(d->URLRest) + d->EndPoints().value(d->URLUserDataStream)),
                                                                               params, false, false),
                                                                ""
                                                                );
        if ( !reply )
            return;
        QMetaObject::Connection c;
        c = connect(reply, &QNetworkReply::finished, [=](){
            if ( reply && reply->error() == QNetworkReply::NoError )
                reply->deleteLater();
            disconnect(c);
        });
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &BinanceAPI::httpError);
    } else if( e->timerId() == m_userWSPingTimerId ) {
        if ( m_webSocketUserDataClient->isOpen() )
            m_webSocketUserDataClient->ping();
        if ( m_webSocketMarketsClient->isOpen() )
            m_webSocketMarketsClient->ping();
    }
}

BinanceAPI::~BinanceAPI() noexcept
{
    this->stop();
}

void BinanceAPI::start() noexcept
{
    if ( !m_webSocketMarketsClient->isConnected() )
        m_webSocketMarketsClient->open();
    if (m_userWSPingTimerId == 0)
        m_userWSPingTimerId = startTimer(10000);
}

void BinanceAPI::stop() noexcept
{
    this->disconnect(SIGNAL(disconnectedPublicApi())); //FIXME: возможно нужно переносить в другое место
    this->disconnect(SIGNAL(disconnectedPrivateApi()));
    if ( m_webSocketMarketsClient->isConnected() )
        m_webSocketMarketsClient->close();
    logOut();
    if ( m_webSocketUserDataClient->isConnected() )
        m_webSocketUserDataClient->close();
    deleteToken();
}

void BinanceAPI::processExchangeInfo()
{
    Q_D(const BinanceAPI);

    std::function getData = [this](){
        if ( SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
#ifndef QT_NO_DEBUG
            const QString testDataFilename{QCoreApplication::instance()->applicationDirPath() + QStringLiteral("/test_data/exchangeInfo.json")};
#else
            const QString testDataFilename{QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QString(QStringLiteral("/.%1/test_data/exchangeInfo.json")).arg(GLOBAL_APP_NAME)};
#endif
            QFile testData(testDataFilename);

            if ( testData.open(QIODevice::ReadOnly) ) {
                const auto &bytes = QJsonDocument::fromJson(testData.readAll());
                testData.close();
                return bytes.object()[QStringLiteral("symbols")].toArray();
            } else
                LOG_WARNING(QString("Error opening test data file=%1, :%2").arg(testDataFilename).arg(testData.errorString()));

            return QJsonArray();
        } else {
            QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
            return processingReply(reply).object()[QStringLiteral("symbols")].toArray();
        }
    };

    const auto &array = getData();
    QList<Currency::Config> currencys;
    QList<exchange::MarketEntry> markets;
    //NOTE: обход по самим монетам
    for (const QJsonValue &v : array) {
        const QJsonObject &obj = v.toObject();
        const QString &strStatus = obj[QStringLiteral("status")].toString();
        const Currency::CurrencyStatus stat = currencyStatusToEnum(strStatus);

        const auto &baseAsset = obj[QStringLiteral("baseAsset")].toString();

        auto createCoin = [&](const QString &symbol, Currency::CurrencyStatus stat)  {
            Currency::Config coin;
            coin.id = Currency::CurrencyId(symbol);

            auto found = std::find_if(currencys.cbegin(), currencys.cend(), [&coin](const Currency::Config &item){
                return coin.id == item.id;
            });

            if ( found == currencys.cend() ) {
                coin.status = stat;
                coin.listingStatus = currencyListingStatusToEnum(strStatus);
                currencys << coin;
            }
        };

        createCoin(baseAsset, stat);
        const auto &quoteAsset = obj[QStringLiteral("quoteAsset")].toString();
        createCoin(quoteAsset, stat);
    }
    if ( !currencys.isEmpty() )
        emit receiveCurrencies(currencys);

    //NOTE: обход по рынкам
    for (const QJsonValue &v : array) {
        const QJsonObject &obj = v.toObject();
        const Market::MarketStatus stat = marketStatusToEnum(obj[QStringLiteral("status")].toString());
        const auto &baseAsset = obj[QStringLiteral("baseAsset")].toString();
        const auto &quoteAsset = obj[QStringLiteral("quoteAsset")].toString();

        exchange::MarketEntry market;

        market.tradeCurrencyId = Market::MarketId(baseAsset);
        market.baseCurrencyId = Market::MarketId(quoteAsset);
        market.params.id = Market::MarketId(obj[QStringLiteral("symbol")].toString());
        market.params.status = stat;
        market.params.tradeFee = BINANCE_FEE;

        const auto &filters = obj[QStringLiteral("filters")].toArray();
        for (const QJsonValue &x : filters) {
            const QJsonObject &filter = x.toObject();
            switch (d->SymbolFiltersMap().key(filter.value(QStringLiteral("filterType")).toString())) {
            case BinanceAPIPrivate::PRICE_FILTER: {
                market.params.minBaseTradePrice = CryptoShloma::Utils::toDouble(filter.value(QStringLiteral("minPrice")));
                market.params.maxBaseTradePrice = CryptoShloma::Utils::toDouble(filter.value(QStringLiteral("maxPrice")));
                market.params.priceTickSize = CryptoShloma::Utils::toDouble(filter.value(QStringLiteral("tickSize")));
                break;
            }
            case BinanceAPIPrivate::PERCENT_PRICE: {
                //TODO: добавить проверку на этот фильтр
//                    market.params.minBaseTradePrice = CryptoShloma::Utils::toDouble(filter.value(QStringLiteral("minPrice")));
//                    market.params.maxBaseTradePrice = CryptoShloma::Utils::toDouble(filter.value(QStringLiteral("maxPrice")));
//                    market.params.priceTickSize = CryptoShloma::Utils::toDouble(filter.value(QStringLiteral("tickSize")));
                break;
            }
            case BinanceAPIPrivate::LOT_SIZE: {
                market.params.minBaseVolume = CryptoShloma::Utils::toDouble(filter.value(QStringLiteral("minQty")));
                market.params.maxBaseVolume = CryptoShloma::Utils::toDouble(filter.value(QStringLiteral("maxQty")));
                market.params.volumeStepSize = CryptoShloma::Utils::toDouble(filter.value(QStringLiteral("stepSize")));
                break;
            }
            case BinanceAPIPrivate::MIN_NOTIONAL: {
                market.params.minNotional = CryptoShloma::Utils::toDouble(filter.value(QStringLiteral("minNotional")));
                break;
            }
            default: break;
            }
        }
        markets.append(market);
    }

    if ( !markets.isEmpty() )
        emit receiveMarketInfo(markets);
}

void BinanceAPI::processMarketData()
{
    Q_D(BinanceAPI);
    std::function getData = [this](){
        if ( SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
#ifndef QT_NO_DEBUG
            const QString testDataFilename{QCoreApplication::instance()->applicationDirPath() + QStringLiteral("/test_data/24hr.json")};
#else
            const QString testDataFilename{QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QString(QStringLiteral("/.%1/test_data/24hr.json")).arg(GLOBAL_APP_NAME)};
#endif
            QFile testData(testDataFilename);

            if ( testData.open(QIODevice::ReadOnly) ) {
                const auto &bytes = QJsonDocument::fromJson(testData.readAll());
                testData.close();
                return bytes.array();
            } else
                LOG_WARNING(QString("Error opening test data file=%1, :%2").arg(testDataFilename).arg(testData.errorString()));

            return QJsonArray();
        } else {
            QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
            return processingReply(reply).array();
        }
    };

    const auto &array = getData();
    LOG_CDEBUG("BinanceAPI.net") << QJsonDocument(array).toJson().constData();
    QList<exchange::MarketEntry> markets;

    //NOTE: обход по массиву
    for (const QJsonValue &v : array) {
        const QJsonObject &obj = v.toObject();
        const QString &symbol = obj.value(QStringLiteral("symbol")).toString();

        exchange::MarketEntry deltaMarket;
        deltaMarket.params.id = Market::MarketId(symbol);
        deltaMarket.params.low = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("lowPrice")));
        deltaMarket.params.high = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("highPrice")));
        deltaMarket.params.bidPrice = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("bidPrice")));
        deltaMarket.params.askPrice = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("askPrice")));
        deltaMarket.params.open = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("openPrice")));
        deltaMarket.params.volume = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("volume")));
        deltaMarket.params.baseVolume = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("quoteVolume")));
        deltaMarket.params.lastPrice = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("lastPrice")));
        deltaMarket.params.tradeFee = BINANCE_FEE;
        calculateSpreadForMarket(deltaMarket.params);
        markets << deltaMarket;
    }
    if ( !markets.isEmpty() )
        emit receiveMarketData(markets);

    if ( !SettingsManager::get(SettingsManager::EnableTestData).toBool() )
        m_webSocketMarketsClient->addStreamTemplate(d->StreamsEndPoints().value(d->AllMarketTickersStream));
}

void BinanceAPI::processStreams(const QString &message)
{
    Q_D(BinanceAPI);
    const QJsonDocument &doc = QJsonDocument::fromJson(message.toUtf8());
    const QJsonObject &root = doc.object();

    const auto &streamName = root.value(QStringLiteral("stream")).toString();

    if ( streamName.contains(d->StreamsEndPoints().value(d->AllMarketTickersStream)) )
        processAllMarketTicketStream(root);
    else if ( streamName.contains(d->StreamsEndPoints().value(d->PartialBookDepthStreams)) )
        processPartialBookDepthStream(root);
    else
        LOG_WARNING(QString("Receive not supported market stream event: %1").arg(streamName));
}

void BinanceAPI::processUserStreams(const QString &message)
{
    Q_D(BinanceAPI);
    const QJsonDocument &doc = QJsonDocument::fromJson(message.toUtf8());
    const QJsonObject &root = doc.object();

    const auto &streamName = root.value(QStringLiteral("e")).toString();

    if ( streamName.contains(d->StreamsEndPoints().value(d->OutboundAccountInfoStream)) ||
         streamName.contains(d->StreamsEndPoints().value(d->OutboundAccountPositionStream)) )
        processOutboundAccountInfo(root);
    else if ( streamName.contains(d->StreamsEndPoints().value(d->ExecutionReportStream)) )
        processExecutionReportStream(root);
    else
        LOG_WARNING(QString("Receive not supported user stream event: %1").arg(streamName));
}

void BinanceAPI::processBalance()
{
    std::function getData = [this](){
        if ( SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
#ifndef QT_NO_DEBUG
            const QString testDataFilename{QCoreApplication::instance()->applicationDirPath() + QStringLiteral("/test_data/balance.json")};
#else
            const QString testDataFilename{QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QString(QStringLiteral("/.%1/test_data/balance.json")).arg(GLOBAL_APP_NAME)};
#endif
            QFile testData(testDataFilename);

            if ( testData.open(QIODevice::ReadOnly) ) {
                const auto &bytes = QJsonDocument::fromJson(testData.readAll());
                testData.close();
                return bytes.object();
            } else
                LOG_WARNING(QString("Error opening test data file=%1, :%2").arg(testDataFilename).arg(testData.errorString()));

            return QJsonObject();
        } else {
            QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
            return processingReply(reply).object();
        }
    };
    const auto &root = getData();

    if ( root.isEmpty() )
        return;
    LOG_CDEBUG("BinanceAPI.net") << QJsonDocument(root).toJson().constData();
    const auto &array = root[QStringLiteral("balances")].toArray();

    QList<exchange::BalanceEntry> balances;

    //NOTE: обход по массиву
    for (const QJsonValue &v : array) {
        const QJsonObject &obj = v.toObject();
        const QString &symbol = obj.value(QStringLiteral("asset")).toString();

        exchange::BalanceEntry balance;
        balance.currencyId = Currency::CurrencyId(symbol);
        balance.params.heldForTrades = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("locked")));
        balance.params.available = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("free")));
        balance.params.total = balance.params.heldForTrades + balance.params.available;
        balance.params.timestamp = QDateTime::fromMSecsSinceEpoch(root.value(QStringLiteral("updateTime")).toVariant().toLongLong());
        balances << balance;
    }
    if ( !balances.isEmpty() )
        emit receiveBalance(balances);
}

void BinanceAPI::processUserTradeHistory()
{
    std::function getData = [this](){
    if ( SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
#ifndef QT_NO_DEBUG
            const QString testDataFilename{QCoreApplication::instance()->applicationDirPath() + QStringLiteral("/test_data/myTrades.json")};
#else
            const QString testDataFilename{QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QString(QStringLiteral("/.%1/test_data/myTrades.json")).arg(GLOBAL_APP_NAME)};
#endif
            QFile testData(testDataFilename);

            if ( testData.open(QIODevice::ReadOnly) ) {
                const auto &bytes = QJsonDocument::fromJson(testData.readAll());
                testData.close();
                return bytes.array();
            } else
                LOG_WARNING(QString("Error opening test data file=%1, :%2").arg(testDataFilename).arg(testData.errorString()));

            return QJsonArray();
        } else {
            QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
            return processingReply(reply).array();
        }
    };

    const auto &array = getData();
    LOG_CDEBUG("BinanceAPI.net") << QJsonDocument(array).toJson().constData();
    QList<exchange::OrderEntry> orders;

    //NOTE: обход по массиву
    for (const QJsonValue &v : array) {
        const QJsonObject &obj = v.toObject();
        const auto &orderId = Order::OrderId(obj.value(QStringLiteral("orderId")).toVariant().toULongLong());

        auto found = std::find_if(orders.begin(), orders.end(), [&orderId](const exchange::OrderEntry &entry) {
            return entry.params.orderId == orderId;
        });

        const qreal &currencyVolume = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("qty")));
        const qreal &baseVolume = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("quoteQty")));
        if ( found != orders.end() ) {
            found->params.currencyVolume += currencyVolume;
            found->params.baseVolume += baseVolume;
            continue;
        }

        const auto &symbol = obj.value(QStringLiteral("symbol")).toString();

        exchange::OrderEntry order;
        order.marketId = Market::MarketId(symbol);
        order.params.orderId = orderId;
        order.params.price = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("price")));
        order.params.currencyVolume = currencyVolume;
        order.params.baseVolume = baseVolume;
        order.params.fee = order.params.baseVolume * BINANCE_FEE;//CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("commission")));
        order.params.timeStamp = QDateTime::fromMSecsSinceEpoch(obj.value(QStringLiteral("time")).toVariant().toLongLong());
        order.params.updatedTimeStamp = order.params.timeStamp;
        order.params.type = obj.value(QStringLiteral("isBuyer")).toBool() ? Order::Buy : Order::Sell;
        orders << order;
    }

    //WARNING: костыль, если у нас в списке много ордеров, то считаем, что это просто запрос по истории и во временной таблице их не может быть
    if ( orders.count() == 1) {
        const auto &order = orders.first();
        LOG_TRACE("call erase order");
        tempOrders()->updateEraseOrder(TemporaryOrdersTable::OrderRecord(order.marketId,
                                                                         order.params.orderId,
                                                                         order.params.timeStamp,
                                                                         order.params.price,
                                                                         order.params.baseVolume,
                                                                         order.params.type,
                                                                         order.params.currencyVolume));
    }


    if ( !orders.isEmpty() )
        emit receiveUserTradeHistory(orders);
}

void BinanceAPI::processUserOpenOrders()
{
    Q_D(BinanceAPI);
    std::function getData = [this](){
        if ( SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
#ifndef QT_NO_DEBUG
            const QString testDataFilename{QCoreApplication::instance()->applicationDirPath() + QStringLiteral("/test_data/allOrders.json")};
#else
            const QString testDataFilename{QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QString(QStringLiteral("/.%1/test_data/allOrders.json")).arg(GLOBAL_APP_NAME)};
#endif
            QFile testData(testDataFilename);

            if ( testData.open(QIODevice::ReadOnly) ) {
                const auto &bytes = QJsonDocument::fromJson(testData.readAll());
                testData.close();
                return bytes.array();
            } else
                LOG_WARNING(QString("Error opening test data file=%1, :%2").arg(testDataFilename).arg(testData.errorString()));

            return QJsonArray();
        } else {
            QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
            return processingReply(reply).array();
        }
    };

    const auto &array = getData();
    LOG_CDEBUG("BinanceAPI.net") << QJsonDocument(array).toJson().constData();
    QList<exchange::OrderEntry> openOrders;

    //NOTE: обход по массиву
    for (const QJsonValue &v : array) {
        const QJsonObject &obj = v.toObject();
        const auto &symbol = obj.value(QStringLiteral("symbol")).toString();

        exchange::OrderEntry order;
        order.marketId = Market::MarketId(symbol);
        order.params.orderId = Order::OrderId(obj.value(QStringLiteral("orderId")).toVariant().toULongLong());
        order.params.price = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("price")));
        order.params.currencyVolume = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("origQty")));
        order.params.baseVolume = order.params.currencyVolume * order.params.price; //TODO: можно заменить из данных с биржи
        order.params.remaining = order.params.currencyVolume - CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("executedQty")));
        order.params.timeStamp = QDateTime::fromMSecsSinceEpoch(obj.value(QStringLiteral("time")).toVariant().toLongLong());
        order.params.updatedTimeStamp = QDateTime::fromMSecsSinceEpoch(obj.value(QStringLiteral("updateTime")).toVariant().toLongLong());
        order.params.type = orderTypeToEnum(obj.value(QStringLiteral("side")).toString());

        const auto &status = obj.value(QStringLiteral("status")).toString();
        if ( !status.compare(d->OrderStatusMap().value(BinanceAPIPrivate::PARTIALLY_FILLED)) ||
             !status.compare(d->OrderStatusMap().value(BinanceAPIPrivate::NEW)) )
            openOrders << order;
    }

    //WARNING: костыль, если у нас в списке много ордеров, то считаем, что это просто запрос по истории открытых ордеров и во временной таблице их не может быть
    if ( openOrders.count() == 1) {
        const auto &order = openOrders.first();
        tempOrders()->updateEraseOrder(TemporaryOrdersTable::OrderRecord(order.marketId,
                                                                         order.params.orderId,
                                                                         order.params.timeStamp,
                                                                         order.params.price,
                                                                         order.params.baseVolume,
                                                                         order.params.type,
                                                                         order.params.currencyVolume));
    }

    if ( !openOrders.isEmpty() )
        emit receiveUserOpenOrders(openOrders);
}

void BinanceAPI::processCancelOrders()
{
    LOG_TRACE_TIME();
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    const auto &object = processingReply(reply).object();
    if ( object.isEmpty() )
        return;
    QList<Order::OrderId> removedOrders;
    const auto &orderId = Order::OrderId(object.value(QStringLiteral("orderId")).toVariant().toULongLong());
    removedOrders << orderId;
    LOG_TRACE("call erase order by Id");
    tempOrders()->updateEraseOrder(orderId);

    if ( !removedOrders.isEmpty() )
        emit receiveCanceledOrders(removedOrders);
}

void BinanceAPI::processNewOrders()
{
    Q_D(BinanceAPI);
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    const auto &rep = processingReply(reply);
    const auto &object = rep.object();

    if ( object.isEmpty() )
        return;
    const Market::MarketId &marketId = Market::MarketId(object.value(QStringLiteral("symbol")).toString());
    const Order::OrderId &orderId = Order::OrderId(object.value(QStringLiteral("orderId")).toVariant().toULongLong());
    const qreal &price = CryptoShloma::Utils::toDouble(object.value(QStringLiteral("price")));
    const qreal &volume = CryptoShloma::Utils::toDouble(object.value(QStringLiteral("origQty")));
    const qreal &baseVolume = volume * price;
    const Order::OrderType type = orderTypeToEnum(object.value(QStringLiteral("side")).toString());
    const QDateTime &timestamp = QDateTime::fromMSecsSinceEpoch(object.value(QStringLiteral("transactTime")).toVariant().toLongLong());
    const auto &status = object.value(QStringLiteral("status")).toString();

    const auto &record = TemporaryOrdersTable::OrderRecord(marketId, orderId, timestamp, price, baseVolume, type, volume);

    if ( !status.compare(d->OrderStatusMap().value(BinanceAPIPrivate::PARTIALLY_FILLED)) ||
         !status.compare(d->OrderStatusMap().value(BinanceAPIPrivate::NEW)) ||
         !status.compare(d->OrderStatusMap().value(BinanceAPIPrivate::FILLED)))
        tempOrders()->updatePlaceOrder(record);
    else {
        LOG_CDEBUG("BinanceAPI.net") << rep.toJson().constData();
        LOG_TRACE("call erase order");
        tempOrders()->updateEraseOrder(record);
    }
}

void BinanceAPI::processUserAuth()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    const auto &object = processingReply(reply).object();
    if ( object.isEmpty() )
        return;
    m_userToken = object.value(QStringLiteral("listenKey")).toString();

    if ( !m_userToken.isEmpty() && !m_userToken.isNull()) {
        m_webSocketUserDataClient->addStreamTemplate(m_userToken);
        if ( !m_webSocketUserDataClient->isConnected() )
            m_webSocketUserDataClient->open();
        if ( !m_userWSPutKeyTimerId )
            m_userWSPutKeyTimerId = startTimer(1000 * 180);
    }
}

QJsonDocument BinanceAPI::processingReply(QNetworkReply *reply) noexcept
{
    if ( !reply )
        return {};
    if (reply->error() == QNetworkReply::NoError) {
        if ( !m_isOnline ) {
            m_isOnline = true;
            emit isOnlineChanged();
        }
        const QJsonDocument &doc = QJsonDocument::fromJson(reply->readAll());
        reply->deleteLater();
        return doc;
    } else return {};
}

QString BinanceAPI::cancelOrderTypeToString(const IExchangeAPI::CancellationOrderType type) const noexcept
{
    Q_UNUSED(type)
    return {};
}

Market::MarketStatus BinanceAPI::marketStatusToEnum(const QString &status) const noexcept
{
    Q_D(const BinanceAPI);
    auto val = d->SymbolStatusMap().key(status);

    switch (val) {
        case BinanceAPIPrivate::SymbolStatus::TRADING: return Market::Ok;
        default: return Market::Closed;
    }
}

QString BinanceAPI::marketStatusToString(const Market::MarketStatus status) const noexcept
{
    Q_UNUSED(status)
    return {};
}

Currency::CurrencyStatus BinanceAPI::currencyStatusToEnum(const QString &status) const noexcept
{
    Q_D(const BinanceAPI);
    auto val = d->SymbolStatusMap().key(status);

    switch (val) {
    case BinanceAPIPrivate::SymbolStatus::PRE_TRADING:
    case BinanceAPIPrivate::SymbolStatus::TRADING:
    case BinanceAPIPrivate::SymbolStatus::POST_TRADING:
        return Currency::Ok;
    case BinanceAPIPrivate::SymbolStatus::HALT:
        return Currency::Maintenance;
    default: return Currency::Offline;
    }
}

QString BinanceAPI::currencyStatusToString(const Currency::CurrencyStatus status) const noexcept
{
    Q_UNUSED(status)
    return {};
}

Currency::CurrencyStatusListing BinanceAPI::currencyListingStatusToEnum(const QString &status) const noexcept
{
    Q_D(const BinanceAPI);
    auto val = d->SymbolStatusMap().key(status);

    switch (val) {
    case BinanceAPIPrivate::SymbolStatus::HALT:
    case BinanceAPIPrivate::SymbolStatus::BREAK:
        return Currency::Delisting;
    default: return Currency::Active;
    }
}

QString BinanceAPI::currencyListingStatusToString(const Currency::CurrencyStatusListing status) const noexcept
{
    Q_UNUSED(status)
    return {};
}

Balance::BalanceStatus BinanceAPI::balanceStatusToEnum(const QString &status) const noexcept
{
    Q_UNUSED(status)
    return {};
}

QString BinanceAPI::balanceStatusToString(const Balance::BalanceStatus status) const noexcept
{
    Q_UNUSED(status)
    return {};
}

Order::OrderType BinanceAPI::orderTypeToEnum(const QString &type) const noexcept
{
    Q_D(const BinanceAPI);

    switch (d->OrderSideMap().key(type)) {
    case BinanceAPIPrivate::BUY: return Order::Buy;
    default: return Order::Sell;
    }
}

QString BinanceAPI::orderTypeToString(const Order::OrderType type) const noexcept
{
    Q_D(const BinanceAPI);

    switch (type) {
    case Order::Buy: return d->OrderSideMap().value(BinanceAPIPrivate::BUY);
    default: return d->OrderSideMap().value(BinanceAPIPrivate::SELL);
    }
}

void BinanceAPI::userAuth() const
{
    Q_D(const BinanceAPI);

    QEventLoop loop;
    QByteArray params;
    auto reply = !m_requestsIsBlock ? const_cast<WebClient *>(m_webClient)->post(prepareMbxRequest(SettingsManager::get(SettingsManager::BinanceAPIKey, SettingsManager::Binance).toString(),
                                                                           SettingsManager::get(SettingsManager::BinanceSecretKey, SettingsManager::Binance).toString(),
                                                                           QUrl(d->EndPoints().value(d->URLRest) + d->EndPoints().value(d->URLUserDataStream)),
                                                                           params, true),
                                                            ""
                                                            ) : nullptr;

    if ( !reply )
        return;
    reply->setProperty(d->replyProperty, __func__);
    connect(reply, &QNetworkReply::finished, this, &BinanceAPI::processUserAuth);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &BinanceAPI::httpError);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
}

void BinanceAPI::listCurrencies() const
{
    //Не требуется реализация
}

void BinanceAPI::listMarketInfo() const
{
    Q_D(const BinanceAPI);

    if ( SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
        const_cast<BinanceAPI*>( this )->processExchangeInfo(); //FIXME: const_cast
        return;
    }

    QEventLoop loop;
    auto reply = !m_requestsIsBlock ? const_cast<WebClient *>(m_webClient)->get(QNetworkRequest(QUrl(d->EndPoints().value(d->URLRest) + d->EndPoints().value(d->URLExchangeInfo)))) : nullptr;
    if ( !reply )
        return;
    connect(reply, &QNetworkReply::finished, this, &BinanceAPI::processExchangeInfo);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &BinanceAPI::httpError);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &BinanceAPI::disconnectedPublicApi);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
}

void BinanceAPI::listMarketData(const Market::MarketId &baseMarket, const quint16 hours) const
{
    Q_UNUSED(baseMarket)
    Q_UNUSED(hours)
    Q_D(const BinanceAPI);

    if ( SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
        const_cast<BinanceAPI*>( this )->processMarketData(); //FIXME: const_cast
        return;
    }

    QEventLoop loop;
    auto reply = !m_requestsIsBlock ? const_cast<WebClient *>(m_webClient)->get(QNetworkRequest(QUrl(d->EndPoints().value(d->URLRest) + d->EndPoints().value(d->URL24TickerPriceChangeStatistics)))) : nullptr;
    if ( !reply )
        return;
    connect(reply, &QNetworkReply::finished, this, &BinanceAPI::processMarketData);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &BinanceAPI::httpError);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &BinanceAPI::disconnectedPublicApi);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
}

void BinanceAPI::listMarketOrders(const Market::MarketId &id, const quint16 count) const
{
    Q_D(const BinanceAPI);
    const QString &marketOrderStream = id.get().toLower() + d->StreamsEndPoints().value(d->PartialBookDepthStreams) + QString::number(count);
    m_webSocketMarketsClient->addStreamTemplate(marketOrderStream);
}

void BinanceAPI::removeListMarketOrders(const Market::MarketId &marketId)
{
    Q_D(const BinanceAPI);
    const QString &marketOrderStream = marketId.get().toLower() + d->StreamsEndPoints().value(d->PartialBookDepthStreams) + QString::number(CONST_ORDERS_COUNT);
    m_webSocketMarketsClient->removeStreamTemplate(marketOrderStream);
}

void BinanceAPI::listMarketOrderGroups(const QList<Market::MarketId> &markets, const quint16 count) const
{
    Q_UNUSED(count)
    Q_D(const BinanceAPI);
    QStringList streamTemplate;
    for ( const auto &item : markets ) {
        const QString &marketOrderStream = item.get().toLower() + d->StreamsEndPoints().value(d->PartialBookDepthStreams) + QString::number(count);
        streamTemplate << marketOrderStream;
    }
    m_webSocketMarketsClient->addStreamTemplate(streamTemplate.join("/"));
}

void BinanceAPI::userBalance(const Currency::CurrencyId &currencyId) const
{
    Q_UNUSED(currencyId)
    Q_D(const BinanceAPI);

    if ( SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
        const_cast<BinanceAPI*>( this )->processBalance(); //FIXME: const_cast
        return;
    }

    QByteArray params;
    auto reply = !m_requestsIsBlock ? const_cast<WebClient *>(m_webClient)->get(prepareMbxRequest(SettingsManager::get(SettingsManager::BinanceAPIKey, SettingsManager::Binance).toString(),
                                                                            SettingsManager::get(SettingsManager::BinanceSecretKey, SettingsManager::Binance).toString(),
                                                                            QUrl(d->EndPoints().value(d->URLRest) + d->EndPoints().value(d->URLAccountInformation)),
                                                                            params)
                                                          ) : nullptr;

    if ( !reply )
        return;
    reply->setProperty(d->replyProperty, __func__);
    connect(reply, &QNetworkReply::finished, this, &BinanceAPI::processBalance);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &BinanceAPI::httpError);
}

void BinanceAPI::userOpenOrders(const Market::MarketId &market, const quint16 count) const
{
    Q_UNUSED(count)
    Q_D(const BinanceAPI);

    if ( SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
        const_cast<BinanceAPI*>( this )->processUserOpenOrders(); //FIXME: const_cast
        return;
    }
    QEventLoop loop;
    QByteArray params;
    if ( market.isValid() )
        params.append(QString("symbol=%1").arg(market.get()));

    auto reply = !m_requestsIsBlock ? const_cast<WebClient *>(m_webClient)->get(prepareMbxRequest(SettingsManager::get(SettingsManager::BinanceAPIKey, SettingsManager::Binance).toString(),
                                                                           SettingsManager::get(SettingsManager::BinanceSecretKey, SettingsManager::Binance).toString(),
                                                                           QUrl(d->EndPoints().value(d->URLRest) + d->EndPoints().value(d->URLCurrentOpenOrders)),
                                                                           params)
                                                            ) : nullptr;

    if ( !reply )
        return;
    reply->setProperty(d->replyProperty, __func__);
    connect(reply, &QNetworkReply::finished, this, &BinanceAPI::processUserOpenOrders);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &BinanceAPI::httpError);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
}

void BinanceAPI::submitOrder(const Market::MarketId &id, const Order::OrderType side, const qreal &price, const qreal &baseVolume, const qreal &currencyVolume)
{
    Q_UNUSED(baseVolume)
    Q_D(const BinanceAPI);

    if ( !m_webSocketUserDataClient->isOpen() && !SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
        LOG_WARNING("User data stream is not connected. Waiting...");
        return;
    }

    if ( !id.isValid() ) {
        LOG_WARNING("MarketId is invalid");
        return;
    }

    const auto &market = marketsStorage()->findMarketById(id);
    if ( market.isNull() )
        return;

    const qreal &tickSize = CryptoShloma::Utils::rounding(market->getPriceTickSize(), price);
    qreal stepSize = side == Order::Buy ? CryptoShloma::Utils::roundingCeil(market->getVolumeStepSize(), currencyVolume) : CryptoShloma::Utils::rounding(market->getVolumeStepSize(), currencyVolume);
    qreal baseStepSize = tickSize * stepSize;

    if ( baseStepSize <= market->getMinNotional() ) {
        if ( side == Order::Buy ) {
            LOG_ERROR(QString("Min notional is invalid %1, need of %2").arg(DOUBLE_TO_STR(baseStepSize)).arg(market->getMinNotional()));
            const qreal newBaseMinVolume = market->getMinNotional() * (1 + market->getTradeFee());
            const qreal newMinVolume = newBaseMinVolume / tickSize;
            stepSize = CryptoShloma::Utils::rounding(market->getVolumeStepSize(), newMinVolume);
            baseStepSize = tickSize * stepSize;
        }
    }

    if ( tempOrders()->submitOrder(TemporaryOrdersTable::OrderRecord(id, side, tickSize, baseStepSize, stepSize)) ) {
        if ( SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
            std::random_device r;
            std::default_random_engine e1(r());
            std::uniform_int_distribution<quint64> uniform_dist(1, 9999999);
            Order::OrderId orderId(uniform_dist(e1));
            emit emulSubmitOrder(id, Order::OrderId(uniform_dist(e1)), side, tickSize, baseStepSize, stepSize);
            const auto &record = TemporaryOrdersTable::OrderRecord(id, orderId, QDateTime::currentDateTime(), tickSize, baseStepSize, side, stepSize);
            tempOrders()->updatePlaceOrder(record);
            tempOrders()->updateEraseOrder(record);
            return;
        }
        QByteArray params;

        params.append(QString("symbol=%1&side=%2&price=%3&newOrderRespType=RESULT")
                          .arg(id.get())
                          .arg(orderTypeToString(side))
                          .arg(CryptoShloma::Utils::toDoublePrec8(tickSize))
                      );

        if ( qFuzzyIsNull(currencyVolume) ) { //TODO: отладить в какой валюте мы размещаем ордер
            params.append(QString("&quoteOrderQty=%1&type=%2")
                          .arg(CryptoShloma::Utils::toDoublePrec8(baseStepSize))
                          .arg(d->OrderTypeMap().value(BinanceAPIPrivate::MARKET)));
        } else {
            params.append(QString("&quantity=%1&type=%2&timeInForce=%3")
                          .arg(CryptoShloma::Utils::toDoublePrec8(stepSize))
                          .arg(d->OrderTypeMap().value(BinanceAPIPrivate::LIMIT))
                          .arg(side == Order::Sell ? d->TimeInForceMap().value(BinanceAPIPrivate::GTC) : d->TimeInForceMap().value(BinanceAPIPrivate::FOK) ));
        }
        LOG_INFO(QString(QStringLiteral("Set %1 %2 price=%3, currencyVolume=%4, baseVolume=%5"))
                     .arg(orderTypeToString(side))
                     .arg(market->getLabel())
                     .arg(DOUBLE_TO_STR(tickSize))
                     .arg(DOUBLE_TO_STR(stepSize))
                     .arg(DOUBLE_TO_STR(baseStepSize)));
        auto reply = !m_requestsIsBlock ? const_cast<WebClient *>(m_webClient)->post(prepareMbxRequest(SettingsManager::get(SettingsManager::BinanceAPIKey, SettingsManager::Binance).toString(),
                                                                                 SettingsManager::get(SettingsManager::BinanceSecretKey, SettingsManager::Binance).toString(),
                                                                                 QUrl(d->EndPoints().value(d->URLRest) + d->EndPoints().value(d->URLNewOrder)),
                                                                                 params, true), params
                                                               ) : nullptr;

        if ( !reply )
            return;
        connect(reply, &QNetworkReply::finished, this, &BinanceAPI::processNewOrders);
        QMetaObject::Connection c;
        c = connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
              [&](){
            LOG_TRACE("call erase order by error");
            tempOrders()->updateEraseOrder(TemporaryOrdersTable::OrderRecord(id, side, tickSize, baseStepSize, stepSize));
            disconnect(c);
        });
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &BinanceAPI::httpError);
    } else {
        LOG_WARNING(QString("Try add to dublicate order on market=%1 side=%2 price=%3 volume=%4 baseVolume=%5")
                 .arg(id.get())
                 .arg(orderTypeToString(side))
                 .arg(CryptoShloma::Utils::toDoublePrec8(tickSize))
                 .arg(CryptoShloma::Utils::toDoublePrec8(stepSize))
                 .arg(CryptoShloma::Utils::toDoublePrec8(baseStepSize)));
    }
}

void BinanceAPI::cancelOrder(const IExchangeAPI::CancellationOrderType type, const Order::OrderId &orderId, const Market::MarketId &id)
{
    Q_D(const BinanceAPI);

    if ( !m_webSocketUserDataClient->isOpen() && !SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
        LOG_WARNING("User data stream is not connected. Waiting...");
        return;
    }

    if ( !id.isValid() ) {
        LOG_WARNING("MarketId is invalid");
        return;
    }

    if ( SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
        emit emulCanceLorder(orderId, id);
        return;
    }

    QByteArray params;

    switch (type) {
    case OrderId: {
        params.append(QString("symbol=%1&orderId=%2").arg(id.get()).arg(orderId.get()));
        break;
    }
    default: return;
    }


    auto reply = !m_requestsIsBlock ? const_cast<WebClient *>(m_webClient)->del(prepareMbxRequest(SettingsManager::get(SettingsManager::BinanceAPIKey, SettingsManager::Binance).toString(),
                                                                             SettingsManager::get(SettingsManager::BinanceSecretKey, SettingsManager::Binance).toString(),
                                                                             QUrl(d->EndPoints().value(d->URLRest) + d->EndPoints().value(d->URLCancelOrder)),
                                                                             params)
                                                           ) : nullptr;

    if ( !reply )
        return;
    connect(reply, &QNetworkReply::finished, this, &BinanceAPI::processCancelOrders);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &BinanceAPI::httpError);
}

void BinanceAPI::userTradeHistory(const Market::MarketId &id, const quint16 count) const
{
    Q_UNUSED(count)
    Q_D(const BinanceAPI);

    if ( SettingsManager::get(SettingsManager::EnableTestData).toBool() ) {
        const_cast<BinanceAPI*>( this )->processUserTradeHistory(); //FIXME: const_cast
        return;
    }
    if ( !id.isValid() ) {
        LOG_WARNING("MarketId is invalid");
        return;
    }
    QEventLoop loop;
    QByteArray params;
    params.append(QString("symbol=%1").arg(id.get()));
    auto reply = !m_requestsIsBlock ? const_cast<WebClient *>(m_webClient)->get(prepareMbxRequest(SettingsManager::get(SettingsManager::BinanceAPIKey, SettingsManager::Binance).toString(),
                                                                            SettingsManager::get(SettingsManager::BinanceSecretKey, SettingsManager::Binance).toString(),
                                                                            QUrl(d->EndPoints().value(d->URLRest) + d->EndPoints().value(d->URLAccountTradeList)),
                                                                            params)
                                                          ) : nullptr;

    if ( !reply )
        return;
    reply->setProperty(d->replyProperty, __func__);
    connect(reply, &QNetworkReply::finished, this, &BinanceAPI::processUserTradeHistory);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &BinanceAPI::httpError);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
}

void BinanceAPI::withdraw(const Currency::CurrencyId &currencyId, const qreal &volume, const QString &address) const
{
    Q_UNUSED(currencyId)
    Q_UNUSED(volume)
    Q_UNUSED(address)
    //TODO: доделать вывод на другой баланс
}

void BinanceAPI::processAllMarketTicketStream(const QJsonObject &root) noexcept
{
    const auto &array = root.value(QStringLiteral("data")).toArray();
    QList<exchange::MarketEntry> markets;

    //NOTE: обход по стримам
    for (const QJsonValue &v : array) {
        const QJsonObject &stream = v.toObject();
        const QString &symbol = stream.value(QStringLiteral("s")).toString();

        exchange::MarketEntry deltaMarket;
        deltaMarket.params.id = Market::MarketId(symbol);
        deltaMarket.params.low = CryptoShloma::Utils::toDouble(stream.value(QStringLiteral("l")));
        deltaMarket.params.high = CryptoShloma::Utils::toDouble(stream.value(QStringLiteral("h")));
        deltaMarket.params.bidPrice = CryptoShloma::Utils::toDouble(stream.value(QStringLiteral("b")));
        deltaMarket.params.askPrice =CryptoShloma::Utils::toDouble(stream.value(QStringLiteral("a")));
        deltaMarket.params.open = CryptoShloma::Utils::toDouble(stream.value(QStringLiteral("o")));
        deltaMarket.params.volume = CryptoShloma::Utils::toDouble(stream.value(QStringLiteral("v")));
        deltaMarket.params.baseVolume = CryptoShloma::Utils::toDouble(stream.value(QStringLiteral("q")));
        deltaMarket.params.lastPrice = CryptoShloma::Utils::toDouble(stream.value(QStringLiteral("c")));
        deltaMarket.params.tradeFee = BINANCE_FEE;
        calculateSpreadForMarket(deltaMarket.params);
        markets << deltaMarket;
    }
    if ( !markets.isEmpty() )
        emit receiveMarketData(markets);
}

void BinanceAPI::processPartialBookDepthStream(const QJsonObject &root) noexcept
{
    const auto &marketId = root.value(QStringLiteral("stream")).toString().split("@").at(0).toUpper();
    const auto &data = root.value(QStringLiteral("data")).toObject();
    QList<exchange::OrderEntry> orders;
    const auto &arrayBuy = data[QStringLiteral("bids")].toArray();
    const auto &arraySell = data[QStringLiteral("asks")].toArray();

    for (const QJsonValue &v : arrayBuy) {
        const auto &obj = v.toArray();
        exchange::OrderEntry order;
        order.marketId = Market::MarketId(marketId);
        order.params.price = CryptoShloma::Utils::toDouble(obj.at(0));
        order.params.currencyVolume = CryptoShloma::Utils::toDouble(obj.at(1));
        order.params.baseVolume = order.params.currencyVolume * order.params.price;
        order.params.type = Order::Buy;
        orders.append(order);
    }

    for (const QJsonValue &v : arraySell) {
        const auto &obj = v.toArray();
        exchange::OrderEntry order;
        order.marketId = Market::MarketId(marketId);
        order.params.price = CryptoShloma::Utils::toDouble(obj.at(0));
        order.params.currencyVolume = CryptoShloma::Utils::toDouble(obj.at(1));
        order.params.baseVolume = order.params.currencyVolume * order.params.price;
        order.params.type = Order::Sell;
        orders.append(order);
    }

    if ( !orders.isEmpty() )
        emit receiveMarketOrders(orders);
}

void BinanceAPI::processOutboundAccountInfo(const QJsonObject &root) noexcept
{
    QList<exchange::BalanceEntry> balances;

    const auto &arrayBalances = root[QStringLiteral("B")].toArray();
    for (const QJsonValue &v : arrayBalances) {
        const auto &obj = v.toObject();
        exchange::BalanceEntry balance;
        balance.currencyId = Currency::CurrencyId(obj.value(QStringLiteral("a")).toString());
        balance.params.heldForTrades = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("l")));
        balance.params.available = CryptoShloma::Utils::toDouble(obj.value(QStringLiteral("f")));
        balance.params.total = balance.params.heldForTrades + balance.params.available;
        balance.params.timestamp = QDateTime::fromMSecsSinceEpoch(root.value(QStringLiteral("u")).toVariant().toLongLong());
        balances << balance;
    }
    if ( !balances.isEmpty() )
        emit receiveBalance(balances);
}

void BinanceAPI::processExecutionReportStream(const QJsonObject &root) noexcept
{
    Q_D(BinanceAPI);
//    LOG_CATEGORY("BinanceAPI")
    QList<exchange::OrderEntry> orders;
    exchange::OrderEntry order;
    order.marketId = Market::MarketId(root.value(QStringLiteral("s")).toString());
    order.params.orderId = Order::OrderId(root.value(QStringLiteral("i")).toVariant().toULongLong());
    const qreal &lastPrice = CryptoShloma::Utils::toDouble(root.value(QStringLiteral("L")));
    order.params.price = qFuzzyIsNull(lastPrice) ? CryptoShloma::Utils::toDouble(root.value(QStringLiteral("p"))) : lastPrice;
    order.params.currencyVolume = CryptoShloma::Utils::toDouble(root.value(QStringLiteral("q")));
    order.params.remaining = order.params.currencyVolume - CryptoShloma::Utils::toDouble(root.value(QStringLiteral("z")));
    order.params.timeStamp = QDateTime::fromMSecsSinceEpoch(root.value(QStringLiteral("O")).toVariant().toLongLong());
    order.params.updatedTimeStamp = QDateTime::fromMSecsSinceEpoch(root.value(QStringLiteral("T")).toVariant().toLongLong());
    order.params.type = orderTypeToEnum(root.value(QStringLiteral("S")).toString());

    const auto &status = root.value(QStringLiteral("X")).toString();

    auto record = TemporaryOrdersTable::OrderRecord(order.marketId,
                                                    order.params.orderId,
                                                    order.params.timeStamp,
                                                    order.params.price,
                                                    order.params.baseVolume,
                                                    order.params.type,
                                                    order.params.currencyVolume);

    if ( !status.compare(d->OrderStatusMap().value(BinanceAPIPrivate::PARTIALLY_FILLED)) ||
         !status.compare(d->OrderStatusMap().value(BinanceAPIPrivate::NEW)) ) {
        order.params.baseVolume = order.params.currencyVolume * order.params.price;
        record.baseVolume = order.params.baseVolume;
        orders << order;
        LOG_CDEBUG("BinanceAPI.net") << QJsonDocument(root).toJson().constData();
        LOG_TRACE("call erase order");
        tempOrders()->updateEraseOrder(record);
        emit receiveUserOpenOrders(orders);
    } else if ( !status.compare(d->OrderStatusMap().value(BinanceAPIPrivate::FILLED)) ) {
        order.params.baseVolume = CryptoShloma::Utils::toDouble(root.value(QStringLiteral("Z")));
        order.params.fee = order.params.baseVolume * BINANCE_FEE;
        record.baseVolume = order.params.baseVolume;
        orders << order;
        LOG_CDEBUG("BinanceAPI.net") << QJsonDocument(root).toJson().constData();
        LOG_TRACE("call erase order");
        tempOrders()->updateEraseOrder(record);
        emit receiveUserTradeHistory(orders);
        emit receiveCanceledOrders(QList<Order::OrderId>() << order.params.orderId);
    } else
        emit receiveCanceledOrders(QList<Order::OrderId>() << order.params.orderId);
}

QNetworkRequest BinanceAPI::prepareMbxRequest(const QString &apiKey, const QString &secretKey, const QUrl &url, QByteArray &params, bool requestBody, bool needAppenParams) const
{
    if ( needAppenParams ) {
        if ( !params.isEmpty() )
            params.append("&");
        params.append(QString("timestamp=%1").arg(QDateTime::currentMSecsSinceEpoch() - m_serverTimeOffset)); //FIXME: server time offset
        const QByteArray &requestSignature = QMessageAuthenticationCode::hash(params, secretKey.toUtf8(), QCryptographicHash::Sha256).toHex();
        params.append(QString("&signature=%1").arg(requestSignature.constData()));
    }
    QString endpoint = url.toString();
    if ( !requestBody )
        endpoint.append(params.constData());

    QNetworkRequest request{QUrl(endpoint)};
    request.setRawHeader(QByteArray(QByteArrayLiteral("X-MBX-APIKEY")), apiKey.toUtf8());

    if ( requestBody )
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return request;
}

void BinanceAPI::serverTime() noexcept
{
    Q_D(const BinanceAPI);
    QEventLoop loop; //TODO: убрать луп
    auto reply = !m_requestsIsBlock ? m_webClient->get(QNetworkRequest(QUrl(d->EndPoints().value(d->URLRest) + d->EndPoints().value(d->URLServerTime)))) : nullptr;

    if ( !reply )
        return;
    auto c = connect(reply, &QNetworkReply::finished, [&](){
        if (!reply)
            return;
        const QJsonDocument &doc = QJsonDocument::fromJson(reply->readAll());
        if ( reply->error() == QNetworkReply::NoError )
            reply->deleteLater();
        m_serverTimeOffset = QDateTime::currentMSecsSinceEpoch() - doc.object()[QStringLiteral("serverTime")].toVariant().toLongLong();
        loop.quit();
    });

    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &BinanceAPI::httpError);
    loop.exec();
    disconnect(c);
}

void BinanceAPI::httpError(QNetworkReply::NetworkError code)
{
    Q_D(const BinanceAPI);
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if ( !reply )
        return;
    const QString &retryAfter = reply->rawHeader("Retry-After");

    const QJsonDocument &doc = QJsonDocument::fromJson(reply->readAll());

    for (const auto &header : reply->rawHeaderPairs())
        LOG_DEBUG(QString("%1:%2").arg(header.first.constData()).arg(header.second.constData()));

    const auto &root = doc.object();
    auto error = root[QStringLiteral("msg")].toString();
    const BinanceAPIPrivate::ErrorCode errorCode = static_cast<BinanceAPIPrivate::ErrorCode>(root[QStringLiteral("code")].toVariant().toInt());

    switch (code) {
    case QNetworkReply::HostNotFoundError:
        m_requestsIsBlock = true;
        m_requestBreakTimer->start(3000);
        break;
    default:break;
    }

    switch (errorCode) {
    case BinanceAPIPrivate::ERROR_INVALID_TIMESTAMP:
    {
        serverTime();

        QString methodName = reply->property(d->replyProperty).toString();
        if ( !methodName.isEmpty() ) {
            LOG_DEBUG(QString("Restart request %1").arg(methodName));
            QMetaObject::invokeMethod(this, methodName.toLatin1().constData(), Qt::DirectConnection);
        }
        break;
    }
    case BinanceAPIPrivate::ERROR_TOO_MANY_REQUESTS: {
        m_requestsIsBlock = true;
        m_requestBreakTimer->start(retryAfter.toInt() * 1000);
        break;
    }
    case BinanceAPIPrivate::ERROR_INVALID_LISTEN_KEY: {
        logOut();
        userAuth();
        break;
    }
    default:break;
    }
    reply->deleteLater();

    Crud::getInstance()->executeQuery(QString(QStringLiteral("INSERT INTO errors (text, exchange, timestamp, type) VALUES ('%1', %2, '%3', 'http_error')"))
                                          .arg(error.remove("'"))
                                          .arg(static_cast<qint16>(Exchange::Type::Binance))
                                          .arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
    LOG_WARNING(QString("http error code=%1:%2, binance error code %3:%4")
                .arg(code)
                .arg(reply->errorString())
                .arg(errorCode)
                .arg(error));
    setOffline();
    //FIXME: необходимо решить вопрос с нештаными ошибками соединения
}

void BinanceAPI::logOut()
{
    if ( m_userWSPutKeyTimerId ) {
        killTimer(m_userWSPutKeyTimerId);
        m_userWSPutKeyTimerId = 0;
    }
    if ( m_userWSPingTimerId ) {
        killTimer(m_userWSPingTimerId);
        m_userWSPingTimerId = 0;
    }
    m_webSocketUserDataClient->removeStreamTemplate(QString("listenKey=%1").arg(m_userToken), false);
}

void BinanceAPI::deleteToken()
{
    Q_D(BinanceAPI);
    if ( !m_userToken.isEmpty() ) {
        QEventLoop loop;
        QByteArray params;
        params.append(QString("?listenKey=%1").arg(m_userToken));
        auto reply = !m_requestsIsBlock ? const_cast<WebClient *>(m_webClient)->del(prepareMbxRequest(SettingsManager::get(SettingsManager::BinanceAPIKey, SettingsManager::Binance).toString(),
                                                                               SettingsManager::get(SettingsManager::BinanceSecretKey, SettingsManager::Binance).toString(),
                                                                               QUrl(d->EndPoints().value(d->URLRest) + d->EndPoints().value(d->URLUserDataStream)),
                                                                               params, false, false)
                                                                ) : nullptr;
        if ( !reply )
            return;
        auto c = connect(reply, &QNetworkReply::finished, [&](){
            if (!reply)
                return;
            if ( reply->error() == QNetworkReply::NoError )
                reply->deleteLater();
            m_userToken.clear();
            loop.quit();
        });
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &BinanceAPI::httpError);
        loop.exec();
        disconnect(c);
    }
}
