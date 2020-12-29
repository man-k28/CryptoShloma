#include "binance_api_p.h"

const QMap<BinanceAPIPrivate::SymbolStatus, QString> BinanceAPIPrivate::m_SymbolStatusMap = [] {
    return QMap<BinanceAPIPrivate::SymbolStatus, QString>{
        {PRE_TRADING, QStringLiteral("PRE_TRADING")},
        {TRADING, QStringLiteral("TRADING")},
        {POST_TRADING, QStringLiteral("POST_TRADING")},
        {END_OF_DAY, QStringLiteral("END_OF_DAY")},
        {HALT, QStringLiteral("HALT")},
        {AUCTION_MATCH, QStringLiteral("AUCTION_MATCH")},
        {BREAK, QStringLiteral("BREAK")}
    };
}();

const QMap<BinanceAPIPrivate::OrderStatus, QString> BinanceAPIPrivate::m_OrderStatusMap = [] {
    return QMap<BinanceAPIPrivate::OrderStatus, QString>{
        {NEW, QStringLiteral("NEW")},
        {PARTIALLY_FILLED, QStringLiteral("PARTIALLY_FILLED")},
        {FILLED, QStringLiteral("FILLED")},
        {CANCELED, QStringLiteral("CANCELED")},
        {PENDING_CANCEL, QStringLiteral("PENDING_CANCEL")},
        {REJECTED, QStringLiteral("REJECTED")},
        {EXPIRED, QStringLiteral("EXPIRED")}
    };
}();

const QMap<BinanceAPIPrivate::OrderType, QString> BinanceAPIPrivate::m_OrderTypeMap = [] {
    return QMap<BinanceAPIPrivate::OrderType, QString> {
        {LIMIT, QStringLiteral("LIMIT")},
        {MARKET, QStringLiteral("MARKET")},
        {STOP_LOSS, QStringLiteral("STOP_LOSS")},
        {STOP_LOSS_LIMIT, QStringLiteral("STOP_LOSS_LIMIT")},
        {TAKE_PROFIT, QStringLiteral("TAKE_PROFIT")},
        {TAKE_PROFIT_LIMIT, QStringLiteral("TAKE_PROFIT_LIMIT")},
        {LIMIT_MAKER, QStringLiteral("LIMIT_MAKER")}
    };
}();

const QMap<BinanceAPIPrivate::OrderSide, QString> BinanceAPIPrivate::m_OrderSideMap = [] {
    return QMap<BinanceAPIPrivate::OrderSide, QString>{
        {BUY, QStringLiteral("BUY")},
        {SELL, QStringLiteral("SELL")}
    };
}();

const QMap<BinanceAPIPrivate::TimeInForce, QString> BinanceAPIPrivate::m_TimeInForceMap = [] {
    return QMap<BinanceAPIPrivate::TimeInForce, QString>{
        {GTC, QStringLiteral("GTC")},
        {IOC, QStringLiteral("IOC")},
        {FOK, QStringLiteral("FOK")}
    };
}();

const QMap<BinanceAPIPrivate::Kline, QString> BinanceAPIPrivate::m_KlineMap = [] {
    return QMap<BinanceAPIPrivate::Kline, QString> {
        {Minute1, QStringLiteral("1m")},
        {Minute3, QStringLiteral("3m")},
        {Minute5, QStringLiteral("5m")},
        {Minute15, QStringLiteral("15m")},
        {Minute30, QStringLiteral("30m")},
        {Hour1, QStringLiteral("1h")},
        {Hour2, QStringLiteral("2h")},
        {Hour4, QStringLiteral("4h")},
        {Hour6, QStringLiteral("6h")},
        {Hour8, QStringLiteral("8h")},
        {Hour12, QStringLiteral("12h")},
        {Day1, QStringLiteral("1d")},
        {Day3, QStringLiteral("3d")},
        {Week, QStringLiteral("1w")},
        {Month, QStringLiteral("1M")}
    };
}();

const QMap<BinanceAPIPrivate::RateLimitType, QString> BinanceAPIPrivate::m_RateLimitTypeMap = [] {
    return QMap<BinanceAPIPrivate::RateLimitType, QString>{
        {REQUEST_WEIGHT, QStringLiteral("REQUEST_WEIGHT")},
        {ORDERS, QStringLiteral("IORDERSOC")},
        {RAW_REQUESTS, QStringLiteral("RAW_REQUESTS")}
    };
}();

const QMap<BinanceAPIPrivate::RateLimitIntervals, QString> BinanceAPIPrivate::m_RateLimitIntervalsMap = [] {
    return QMap<BinanceAPIPrivate::RateLimitIntervals, QString>{
        {SECOND, QStringLiteral("SECOND")},
        {MINUTE, QStringLiteral("MINUTE")},
        {DAY, QStringLiteral("DAY")}
    };
}();

const QMap<BinanceAPIPrivate::BinanceUrl, QString> BinanceAPIPrivate::m_EndPoints = [] {
    return QMap<BinanceAPIPrivate::BinanceUrl, QString> {
        {URLRest, QStringLiteral("https://api.binance.com/api/v3/")},
        {URLWithdraw, QStringLiteral("https://api.binance.com/wapi/v3/")},
        {URLWebSockets, QStringLiteral("wss://stream.binance.com:9443/")},
        {URLPing, QStringLiteral("ping")},
        {URLServerTime, QStringLiteral("time")},
        {URLExchangeInfo, QStringLiteral("exchangeInfo")},
        {URLOrderBook, QStringLiteral("depth")}, //web sockets
        {URLRecentTradesList, QStringLiteral("trades")}, //web sockets
        {URLOldTradeLookup, QStringLiteral("historicalTrades")},
        {URLAggregateTradesList, QStringLiteral("aggTrades")}, //web sockets
        {URLKlineData, QStringLiteral("klines")}, //web sockets
        {URLCurrentAveragePrice, QStringLiteral("avgPrice")},
        {URL24TickerPriceChangeStatistics, QStringLiteral("ticker/24hr")},
        {URLSymbolPriceTicker, QStringLiteral("ticker/price")},
        {URLSymbolOrderBookTicker, QStringLiteral("ticker/bookTicker")},
        //Private EndPoints
        {URLNewOrder, QStringLiteral("order")},
        {URLTestNewOrder, QStringLiteral("order/test")},
        {URLQueryOrder, QStringLiteral("order?")},
        {URLCancelOrder, QStringLiteral("order?")},
        {URLCurrentOpenOrders, QStringLiteral("openOrders?")},
        {URLAllOrders, QStringLiteral("allOrders?")},
        {URLNewOCO, QStringLiteral("order/oco")},
        {URLCancelOCO, QStringLiteral("orderList")},
        {URLQueryOCO, QStringLiteral("orderList")},
        {URLQueryAllOCO, QStringLiteral("allOrderList")},
        {URLQueryOpenOCO, QStringLiteral("openOrderList")},
        {URLAccountInformation, QStringLiteral("account?")},
        {URLAccountTradeList, QStringLiteral("myTrades?")},
        {URLUserDataStream, QStringLiteral("userDataStream")},
        {URLAccountWithDraw, QStringLiteral("withdraw.html")}
    };
}();

const QMap<BinanceAPIPrivate::Streams, QString> BinanceAPIPrivate::m_Streams = [] {
    return QMap<BinanceAPIPrivate::Streams, QString> {
        {AggregateTradeStreams, QStringLiteral("@aggTrade")},
        {TradeStreams, QStringLiteral("@trade")},
        {KlineStreams, QStringLiteral("@kline_")},
        {IndividualSymbolMiniTickerStream, QStringLiteral("@miniTicker")},
        {AllMarketMiniTickersStream, QStringLiteral("!miniTicker@arr")},
        {IndividualSymbolTickerStreams, QStringLiteral("@ticker")},
        {AllMarketTickersStream, QStringLiteral("!ticker@arr")},
        {IndividualSymbolBookTickerStreams, QStringLiteral("@bookTicker")},
        {AllBookTickersStream, QStringLiteral("!bookTicker")},
        {PartialBookDepthStreams, QStringLiteral("@depth")},
        {DiffDepthStream, QStringLiteral("@depth")},
        {OutboundAccountInfoStream, QStringLiteral("outboundAccountInfo")},
        {OutboundAccountPositionStream, QStringLiteral("outboundAccountPosition")},
        {ExecutionReportStream, QStringLiteral("executionReport")},
        {ListStatusStream, QStringLiteral("listStatus")},
        {BalanceUpdateStream, QStringLiteral("balanceUpdate")}
    };
}();

const QMap<BinanceAPIPrivate::SymbolFilters, QString> BinanceAPIPrivate::m_SymbolFiltersMap = [] {
    return QMap<BinanceAPIPrivate::SymbolFilters, QString> {
        {PRICE_FILTER, QStringLiteral("PRICE_FILTER")},
        {PERCENT_PRICE, QStringLiteral("PERCENT_PRICE")},
        {LOT_SIZE, QStringLiteral("LOT_SIZE")},
        {MIN_NOTIONAL, QStringLiteral("MIN_NOTIONAL")},
        {ICEBERG_PARTS, QStringLiteral("ICEBERG_PARTS")},
        {MARKET_LOT_SIZE, QStringLiteral("MARKET_LOT_SIZE")},
        {MAX_NUM_ORDERS, QStringLiteral("MAX_NUM_ORDERS")},
        {MAX_NUM_ALGO_ORDERS, QStringLiteral("MAX_NUM_ALGO_ORDERS")},
        {MAX_NUM_ICEBERG_ORDERS, QStringLiteral("MAX_NUM_ICEBERG_ORDERS")},
        {MAX_POSITION, QStringLiteral("MAX_POSITION")}
    };
}();

const QMap<BinanceAPIPrivate::ExchangeFilters, QString> BinanceAPIPrivate::m_ExchangeFiltersMap = [] {
    return QMap<BinanceAPIPrivate::ExchangeFilters, QString> {
        {EXCHANGE_MAX_NUM_ORDERS, QStringLiteral("EXCHANGE_MAX_NUM_ORDERS")},
        {EXCHANGE_MAX_ALGO_ORDERS, QStringLiteral("EXCHANGE_MAX_ALGO_ORDERS")}
    };
}();

//const QMap<BinanceAPIPrivate::ErrorCode, QString> BinanceAPIPrivate::m_ErrorCodeDictionary = [] {
//    return QMap<BinanceAPIPrivate::ErrorCode, QString> {
//        {ERROR_UNKNOWN, QStringLiteral("@aggTrade")}
//    };
//}();

BinanceAPIPrivate::BinanceAPIPrivate(QObject *parent) :
    QObject(parent)
{

}

QMap<BinanceAPIPrivate::SymbolStatus, QString> BinanceAPIPrivate::SymbolStatusMap()
{
    return m_SymbolStatusMap;
}

QMap<BinanceAPIPrivate::OrderStatus, QString> BinanceAPIPrivate::OrderStatusMap()
{
    return m_OrderStatusMap;
}

QMap<BinanceAPIPrivate::OrderType, QString> BinanceAPIPrivate::OrderTypeMap()
{
    return m_OrderTypeMap;
}

QMap<BinanceAPIPrivate::OrderSide, QString> BinanceAPIPrivate::OrderSideMap()
{
    return m_OrderSideMap;
}

QMap<BinanceAPIPrivate::TimeInForce, QString> BinanceAPIPrivate::TimeInForceMap()
{
    return m_TimeInForceMap;
}

QMap<BinanceAPIPrivate::Kline, QString> BinanceAPIPrivate::KlineMap()
{
    return m_KlineMap;
}

QMap<BinanceAPIPrivate::RateLimitType, QString> BinanceAPIPrivate::RateLimitTypeMap()
{
    return m_RateLimitTypeMap;
}

QMap<BinanceAPIPrivate::RateLimitIntervals, QString> BinanceAPIPrivate::RateLimitIntervalsMap()
{
    return m_RateLimitIntervalsMap;
}

QMap<BinanceAPIPrivate::BinanceUrl, QString> BinanceAPIPrivate::EndPoints()
{
    return m_EndPoints;
}

QMap<BinanceAPIPrivate::Streams, QString> BinanceAPIPrivate::StreamsEndPoints()
{
    return m_Streams;
}

QMap<BinanceAPIPrivate::SymbolFilters, QString> BinanceAPIPrivate::SymbolFiltersMap()
{
    return m_SymbolFiltersMap;
}

QMap<BinanceAPIPrivate::ExchangeFilters, QString> BinanceAPIPrivate::ExchangeFiltersMap()
{
    return m_ExchangeFiltersMap;
}
