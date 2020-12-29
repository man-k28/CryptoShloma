#ifndef BINANCE_API_P_H
#define BINANCE_API_P_H
#include "binance_api.h"
#include <QMap>

class BinanceAPIPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(BinanceAPI)
public:
    enum SymbolStatus {
        PRE_TRADING,
        TRADING,
        POST_TRADING,
        END_OF_DAY,
        HALT,
        AUCTION_MATCH,
        BREAK
    };
    Q_ENUM(SymbolStatus)
    enum OrderStatus {
        NEW,
        PARTIALLY_FILLED,
        FILLED,
        CANCELED,
        PENDING_CANCEL, //deprecated
        REJECTED,
        EXPIRED
    };
    Q_ENUM(OrderStatus)
    enum OrderType {
        LIMIT,
        MARKET,
        STOP_LOSS,
        STOP_LOSS_LIMIT,
        TAKE_PROFIT,
        TAKE_PROFIT_LIMIT,
        LIMIT_MAKER
    };
    Q_ENUM(OrderType)

    enum OCOStatus {
        RESPONSE,
        EXEC_STARTED,
        ALL_DONE_Status
    };
    Q_ENUM(OCOStatus)

    enum OCOOrderStatus {
        EXECUTING,
        ALL_DONE_OrderStatus,
        REJECT
    };
    Q_ENUM(OCOOrderStatus)

    enum OrderSide {
        BUY,
        SELL
    };
    Q_ENUM(OrderSide)
    enum TimeInForce {
        GTC,
        IOC,
        FOK,
    };
    Q_ENUM(TimeInForce)
    enum Kline {
        Minute1,
        Minute3,
        Minute5,
        Minute15,
        Minute30,
        Hour1,
        Hour2,
        Hour4,
        Hour6,
        Hour8,
        Hour12,
        Day1,
        Day3,
        Week,
        Month
    };
    Q_ENUM(Kline)
    enum RateLimitType {
        REQUEST_WEIGHT,
        ORDERS,
        RAW_REQUESTS
    };
    Q_ENUM(RateLimitType)
    enum RateLimitIntervals {
        SECOND,
        MINUTE,
        DAY,
    };
    Q_ENUM(RateLimitIntervals)
    enum BinanceUrl {
        URLRest,
        URLWithdraw,
        URLWebSockets,
        //Public EndPoints
        URLPing,
        URLServerTime,
        URLExchangeInfo,
        URLOrderBook,
        URLRecentTradesList,
        URLOldTradeLookup,
        URLAggregateTradesList,
        URLKlineData,
        URLCurrentAveragePrice,
        URL24TickerPriceChangeStatistics,
        URLSymbolPriceTicker,
        URLSymbolOrderBookTicker,
        //Private EndPoints
        URLNewOrder,
        URLTestNewOrder,
        URLQueryOrder,
        URLCancelOrder,
        URLCurrentOpenOrders,
        URLAllOrders,
        URLNewOCO,
        URLCancelOCO,
        URLQueryOCO,
        URLQueryAllOCO,
        URLQueryOpenOCO,
        URLAccountInformation,
        URLAccountTradeList,
        URLUserDataStream,
        URLAccountWithDraw
    };
    Q_ENUM(BinanceUrl)
    enum ErrorCode {
        //10xx - General Server or Network issues
        ERROR_UNKNOWN = -1000, //An unknown error occured while processing the request.
        ERROR_DISCONNECTED = -1001, //Internal error; unable to process your request. Please try again.
        ERROR_UNAUTHORIZED = -1002, //You are not authorized to execute this request.
        ERROR_TOO_MANY_REQUESTS = -1003, //Too many requests queued.
                                   //Too many requests; please use the websocket for live updates.
                                   //Too many requests; current limit is %s requests per minute. Please use the websocket for live updates to avoid polling the API.
                                   //Way too many requests; IP banned until %s. Please use the websocket for live updates to avoid bans.

        ERROR_UNEXPECTED_RESP = -1006, //An unexpected response was received from the message bus. Execution status unknown.
        ERROR_TIMEOUT = -1007, //Timeout waiting for response from backend server. Send status unknown; execution status unknown.
        ERROR_ERROR_MSG_RECEIVED = -1010,
        ERROR_UNKNOWN_ORDER_COMPOSITION = -1014, //Unsupported order combination.
        ERROR_TOO_MANY_ORDERS = -1015, //Too many new orders.
                                 //Too many new orders; current limit is %s orders per %s.
        ERROR_SERVICE_SHUTTING_DOWN = -1016, //This service is no longer available.
        ERROR_UNSUPPORTED_OPERATION = -1020, //This operation is not supported.
        ERROR_INVALID_TIMESTAMP = -1021, //Timestamp for this request is outside of the recvWindow.
                                   //Timestamp for this request was 1000ms ahead of the server's time.
        ERROR_INVALID_SIGNATURE = -1022, //Signature for this request is not valid.
        //11xx - Request issues
        ERROR_ILLEGAL_CHARS = -1100, //Illegal characters found in a parameter.
                               //Illegal characters found in parameter '%s'; legal range is '%s'.

        ERROR_TOO_MANY_PARAMETERS = -1101, //Too many parameters sent for this endpoint.
                                     //Too many parameters; expected '%s' and received '%s'.
                                     //Duplicate values for a parameter detected.
        ERROR_MANDATORY_PARAM_EMPTY_OR_MALFORMED = -1102, //A mandatory parameter was not sent, was empty/null, or malformed.
                                                    //Mandatory parameter '%s' was not sent, was empty/null, or malformed.
                                                    //Param '%s' or '%s' must be sent, but both were empty/null!
        ERROR_UNKNOWN_PARAM = -1103, //An unknown parameter was sent.
        ERROR_UNREAD_PARAMETERS = -1104, //Not all sent parameters were read.
                                   //Not all sent parameters were read; read '%s' parameter(s) but was sent '%s'.
        ERROR_PARAM_EMPTY = -1105, //A parameter was empty.
                             //Parameter '%s' was empty.
        ERROR_PARAM_NOT_REQUIRED = -1106, //A parameter was sent when not required.
                                    //Parameter '%s' sent when not required.
        ERROR_BAD_PRECISION = -1111, //Precision is over the maximum defined for this asset.
        ERROR_NO_DEPTH = -1112, //No orders on book for symbol.
        ERROR_TIF_NOT_REQUIRED = -1114, //TimeInForce parameter sent when not required.
        ERROR_INVALID_TIF = -1115, //Invalid timeInForce.
        ERROR_INVALID_ORDER_TYPE = -1116, //Invalid orderType.
        ERROR_INVALID_SIDE = -1117,
        ERROR_EMPTY_NEW_CL_ORD_ID = -1118, //New client order ID was empty.
        ERROR_EMPTY_ORG_CL_ORD_ID = -1119, //Original client order ID was empty.
        ERROR_BAD_INTERVAL = -1120,
        ERROR_BAD_SYMBOL = -1121,
        ERROR_INVALID_LISTEN_KEY = -1125, //This listenKey does not exist.
        ERROR_MORE_THAN_XX_HOURS = -1127, //Lookup interval is too big.
                                    //More than %s hours between startTime and endTime.
        ERROR_OPTIONAL_PARAMS_BAD_COMBO = -1128, //Combination of optional parameters invalid.
        ERROR_INVALID_PARAMETER = -1130, //Invalid data sent for a parameter.
                                   //Data sent for paramter '%s' is not valid.
        ERROR_NEW_ORDER_REJECTED = -2010,
        ERROR_CANCEL_REJECTED = -2011,
        ERROR_NO_SUCH_ORDER = -2013,
        ERROR_BAD_API_KEY_FMT = -2014, //API-key format invalid.
        ERROR_REJECTED_MBX_KEY = -2015, //Invalid API-key, IP, or permissions for action.
        ERROR_NO_TRADING_WINDOW = -2016, //No trading window could be found for the symbol. Try ticker/24hrs instead.
        //-9xxx Filter failures
        ERROR_PRICE_FILTER = -9000, // price is too high, too low, and/or not following the tick size rule for the symbol.
        ERROR_PERCENT_PRICE = -9001, // price is X% too high or X% too low from the average weighted price over the last Y minutes.
        ERROR_LOT_SIZE = -9002, //quantity is too high, too low, and/or not following the step size rule for the symbol.
        ERROR_MIN_NOTIONAL = -9003, // price * quantity is too low to be a valid order for the symbol.
        ERROR_ICEBERG_PARTS = -9004, // ICEBERG order would break into too many parts; icebergQty is too small.
        ERROR_MARKET_LOT_SIZE = -9005, // MARKET order's quantity is too high, too low, and/or not following the step size rule for the symbol.
        ERROR_MAX_NUM_ORDERS = -9006, // Account has too many open orders on the symbol.
        ERROR_MAX_ALGO_ORDERS = -9007, // Account has too many open stop loss and/or take profit orders on the symbol.
        ERROR_MAX_NUM_ICEBERG_ORDERS = -9008, // Account has too many open iceberg orders on the symbol.
        ERROR_EXCHANGE_MAX_NUM_ORDERS = -9009, // Account has too many open orders on the exchange.
        ERROR_EXCHANGE_MAX_ALGO_ORDERS = -9010, // Account has too many open stop loss and/or take profit orders on the exchange.
        //HTTP error
        ERROR_BREAKING_REQUESTS = 429,
        ERROR_BAN = 418
    };
    Q_ENUM(ErrorCode)
    enum Streams {
        AggregateTradeStreams,
        TradeStreams,
        KlineStreams,
        IndividualSymbolMiniTickerStream,
        AllMarketMiniTickersStream,
        IndividualSymbolTickerStreams,
        AllMarketTickersStream,
        IndividualSymbolBookTickerStreams,
        AllBookTickersStream,
        PartialBookDepthStreams,
        DiffDepthStream,
        //Account stream events
        OutboundAccountInfoStream,
        OutboundAccountPositionStream,
        ExecutionReportStream,
        ListStatusStream,
        BalanceUpdateStream
    };
    Q_ENUM(Streams)

    enum SymbolFilters {
        PRICE_FILTER,
        PERCENT_PRICE,
        LOT_SIZE,
        MIN_NOTIONAL,
        ICEBERG_PARTS,
        MARKET_LOT_SIZE,
        MAX_NUM_ORDERS,
        MAX_NUM_ALGO_ORDERS,
        MAX_NUM_ICEBERG_ORDERS,
        MAX_POSITION
    };
    Q_ENUM(SymbolFilters)

    enum ExchangeFilters {
        EXCHANGE_MAX_NUM_ORDERS,
        EXCHANGE_MAX_ALGO_ORDERS
    };
    Q_ENUM(ExchangeFilters)

    explicit BinanceAPIPrivate(QObject *parent = nullptr);
    explicit BinanceAPIPrivate(const BinanceAPIPrivate &o) noexcept = delete;
    BinanceAPIPrivate& operator=(const BinanceAPIPrivate &o) noexcept = delete;
    BinanceAPI *q_ptr{nullptr};

    static QMap<SymbolStatus, QString> SymbolStatusMap();
    static QMap<OrderStatus, QString> OrderStatusMap();
    static QMap<OrderType, QString> OrderTypeMap();
    static QMap<OrderSide, QString> OrderSideMap();
    static QMap<TimeInForce, QString> TimeInForceMap();
    static QMap<Kline, QString> KlineMap();
    static QMap<RateLimitType, QString> RateLimitTypeMap();
    static QMap<RateLimitIntervals, QString> RateLimitIntervalsMap();
    static QMap<BinanceUrl, QString> EndPoints();
    static QMap<Streams, QString> StreamsEndPoints();
    static QMap<SymbolFilters, QString> SymbolFiltersMap();
    static QMap<ExchangeFilters, QString> ExchangeFiltersMap();
    const char *replyProperty = "methodName";
private:
    static const QMap<SymbolStatus, QString> m_SymbolStatusMap;
    static const QMap<OrderStatus, QString> m_OrderStatusMap;
    static const QMap<OrderType, QString> m_OrderTypeMap;
    static const QMap<OrderSide, QString> m_OrderSideMap;
    static const QMap<TimeInForce, QString> m_TimeInForceMap;
    static const QMap<Kline, QString> m_KlineMap;
    static const QMap<RateLimitType, QString> m_RateLimitTypeMap;
    static const QMap<RateLimitIntervals, QString> m_RateLimitIntervalsMap;
    static const QMap<BinanceUrl, QString> m_EndPoints;
    static const QMap<Streams, QString> m_Streams;
    static const QMap<SymbolFilters, QString> m_SymbolFiltersMap;
    static const QMap<ExchangeFilters, QString> m_ExchangeFiltersMap;
//    static const QMap<ErrorCode, QString> m_ErrorCodeDictionary;
};

#endif // BINANCE_API_P_H
