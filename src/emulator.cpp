#include "emulator.h"
#include <common/constants.h>
#include <Logger.h>
#include <QFileInfo>
#include <api/i_exchange_api.h>
#include <storage/balances_storage.h>
#include <QTextStream>

Emulator::Emulator(IExchangeAPI *parent) noexcept
    : QObject(parent)
{
}

void Emulator::loadMarketOrders(const QUrl &url) noexcept
{
    QFile testData(url.toLocalFile());
    if ( !testData.open(QIODevice::ReadOnly) ) {
        LOG_ERROR(QString("Error of opening test data file: %1. Error: %2").arg(testData.fileName()).arg(testData.errorString()));
        return;
    }

    QFileInfo fileInfo(testData);
    const auto &baseName = fileInfo.baseName().split("_"); //TODO: костыль, завязываться на ID пары не варик, ибо на всех биржах он может быть разного формата
    if ( baseName.count() == 1 || baseName.count() > 2) {
        LOG_ERROR("Invalid test data filename format. Example: XRPBTC_1");
        return;
    }

    QList<exchange::OrderEntry> orders;
    const auto &marketId = Market::MarketId(baseName.at(0));

    QTextStream stream(&testData);
    QString line;
    while (stream.readLineInto(&line)) {
        const auto &data = line.split(",");
        if ( data.count() <= 1 || data.count() > 4 ) {
            LOG_ERROR("Invalid test data format");
            break;
        }

        exchange::OrderEntry sellOrder;
        sellOrder.marketId = marketId;
        sellOrder.params.price = data.at(0).toDouble();
        sellOrder.params.currencyVolume = data.at(1).toDouble();
        sellOrder.params.baseVolume = sellOrder.params.currencyVolume * sellOrder.params.price;
        sellOrder.params.type = Order::Sell;
        orders.append(sellOrder);

        exchange::OrderEntry buyOrder;
        buyOrder.marketId = marketId;
        buyOrder.params.price = data.at(2).toDouble();
        buyOrder.params.currencyVolume = data.at(3).toDouble();
        buyOrder.params.baseVolume = buyOrder.params.currencyVolume * buyOrder.params.price;
        buyOrder.params.type = Order::Buy;
        orders.append(buyOrder);
    }
    testData.close();
    if ( !orders.isEmpty() )
        emit loadedMarketOrders(orders);
}

void Emulator::submitOrder(const Market::MarketId &marketId, const Order::OrderId &orderId, const Order::OrderType side, const qreal &price, const qreal &baseVolume, const qreal &currencyVolume) noexcept
{
    QList<exchange::OrderEntry> orders;
    exchange::OrderEntry order;
    order.marketId = marketId;
    order.params.orderId = orderId;
    order.params.price = price;
    order.params.currencyVolume = currencyVolume;
    order.params.baseVolume = baseVolume;
    order.params.timeStamp = QDateTime::currentDateTime();
    order.params.updatedTimeStamp = order.params.timeStamp;
    order.params.type = side;

    if ( side == Order::Buy ) {
        order.params.fee = order.params.baseVolume * BINANCE_FEE;
        orders << order;
        emit loadedUserTradeHistory(orders);
    } else {
        order.params.remaining = order.params.currencyVolume;
        orders << order;
        emit loadedUserOpenOrders(orders);
    }

    auto balanceStorage = qobject_cast<IExchangeAPI *>(parent())->balancesStorage();
    const auto &currencyId = Currency::CurrencyId(marketId.get().mid(0,3));//TODO: надо подумать, откуда брать ID монеты
    const auto &baseCurrencyId = Currency::CurrencyId(marketId.get().mid(3,3));//TODO: надо подумать, откуда брать ID монеты
    const auto &balanceData = balanceStorage->find(currencyId);
    const auto &baseBalanceData = balanceStorage->find(baseCurrencyId);
    QList<exchange::BalanceEntry> balances;

    if ( balanceStorage && side == Order::Buy ) {
        exchange::BalanceEntry balance;
        balance.currencyId = currencyId;
        balance.params.heldForTrades = balanceData.isNull() ? 0. : balanceData->getHeldForTrades();
        balance.params.available = balanceData.isNull() ? currencyVolume : balanceData->getAvailable() + currencyVolume;
        balance.params.total = balance.params.heldForTrades + balance.params.available;
        balance.params.timestamp = QDateTime::currentDateTime();

        exchange::BalanceEntry baseBalance;
        baseBalance.currencyId = baseCurrencyId;
        baseBalance.params.heldForTrades = baseBalanceData.isNull() ? 0. : baseBalanceData->getHeldForTrades();
        baseBalance.params.available = baseBalanceData.isNull() ? baseVolume : baseBalanceData->getAvailable() - baseVolume;
        baseBalance.params.total = baseBalance.params.heldForTrades + baseBalance.params.available;
        baseBalance.params.timestamp = QDateTime::currentDateTime();

        balances << balance << baseBalance;
    } else {
        exchange::BalanceEntry balance;
        balance.currencyId = currencyId;
        balance.params.heldForTrades = balanceData.isNull() ? 0. : currencyVolume;
        balance.params.available = balanceData.isNull() ? currencyVolume : 0.;
        balance.params.total = balance.params.heldForTrades + balance.params.available;
        balance.params.timestamp = QDateTime::currentDateTime();

        balances << balance;
    }
    emit loadedBalance(balances);
}

void Emulator::cancelOrder(const Order::OrderId &orderId, const Market::MarketId &marketId) noexcept
{
    Q_UNUSED(marketId)
    emit loadedCanceledOrders(QList<Order::OrderId>() << orderId);

    auto balanceStorage = qobject_cast<IExchangeAPI *>(parent())->balancesStorage();

    const auto &currencyId = Currency::CurrencyId(marketId.get().mid(0,3));//TODO: надо подумать, откуда брать ID монеты
    const auto &balanceCurrency = balanceStorage->find(currencyId);

    QList<exchange::BalanceEntry> balances;
    exchange::BalanceEntry balance;
    balance.currencyId = currencyId;
    balance.params.available = balanceCurrency->getHeldForTrades();
    balance.params.heldForTrades = 0.;
    balance.params.total = balanceCurrency->getTotal();
    balance.params.timestamp = QDateTime::currentDateTime();
    balances << balance;

    emit loadedBalance(balances);
}

void Emulator::sellOrder(const Market::MarketId &marketId, const Order::OrderId &orderId, const qreal &price, const qreal &baseVolume, const qreal &currencyVolume) noexcept
{
    QList<Order::OrderId> orders;
    orders << orderId;
    emit loadedCanceledOrders(orders);

    QList<exchange::OrderEntry> ordersEntry;
    exchange::OrderEntry order;
    order.marketId = marketId;
    order.params.orderId = orderId;
    order.params.price = price;
    order.params.currencyVolume = currencyVolume;
    order.params.baseVolume = baseVolume;
    order.params.timeStamp = QDateTime::currentDateTime();
    order.params.updatedTimeStamp = order.params.timeStamp;
    order.params.type = Order::Sell;
    order.params.fee = order.params.baseVolume * BINANCE_FEE;
    ordersEntry << order;
    emit loadedUserTradeHistory(ordersEntry);

    auto balanceStorage = qobject_cast<IExchangeAPI *>(parent())->balancesStorage();

    const auto &currencyId = Currency::CurrencyId(marketId.get().mid(0,3));//TODO: надо подумать, откуда брать ID монеты
    const auto &baseCurrencyId = Currency::CurrencyId(marketId.get().mid(3,3));//TODO: надо подумать, откуда брать ID монеты
    const auto &baseBalanceData = balanceStorage->find(baseCurrencyId);

    QList<exchange::BalanceEntry> balances;
    exchange::BalanceEntry balance;
    balance.currencyId = currencyId;
    balance.params.heldForTrades = 0.;
    balance.params.available = 0.;
    balance.params.total = 0.;
    balance.params.timestamp = QDateTime::currentDateTime();

    exchange::BalanceEntry baseBalance;
    baseBalance.currencyId = baseCurrencyId;
    baseBalance.params.heldForTrades = baseBalanceData.isNull() ? 0. : baseBalanceData->getHeldForTrades();
    baseBalance.params.available = baseBalanceData.isNull() ? baseVolume : baseBalanceData->getAvailable() + baseVolume;
    baseBalance.params.total = baseBalance.params.heldForTrades + baseBalance.params.available;
    baseBalance.params.timestamp = QDateTime::currentDateTime();

    balances << balance << baseBalance;

    emit loadedBalance(balances);
}
