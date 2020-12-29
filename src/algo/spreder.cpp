#include "spreder.h"
#include <QTimerEvent>
#include <QJsonArray>
#include <QJsonObject>
#include "Logger.h"
#include <api/i_exchange_api.h>
#include <common/settings_manager.h>
#include <storage/market_orders_storage.h>
#include <storage/balances_storage.h>
#include <storage/trade_history_storage.h>
#include <storage/open_orders_storage.h>

Spreder::Spreder(QObject *parent) noexcept
    : ITradeAlgorithm(parent)
{
}

bool Spreder::start() noexcept
{
    return true;
}

bool Spreder::stop() noexcept
{
    emit finished();
    return true;
}

bool Spreder::restart() noexcept
{
    return true;
}

void Spreder::reconfigure() noexcept
{

}

void Spreder::timerEvent(QTimerEvent *event) noexcept
{
    Q_UNUSED(event)
}


void Spreder::startSubmitOrders(const Market::ConstPtr &market, const bool sellMode)
{
    LOG_TRACE_TIME();
    Q_ASSERT_X(market.isNull(), "QSharedPointer", "market is nullptr");
    const auto &id = market->getId();
    const QString &currencyName = market->getCurrency()->getId().get();

    //TODO: подумать как можно снизить время на двойную фильтрацию одного и того же
    const auto &buyOrdersFiltered = api()->marketOrdersStorage()->filter(id, Order::OrderType::Buy);
    const auto &sellOrdersFiltered = api()->marketOrdersStorage()->filter(id, Order::OrderType::Sell);
    const auto openBuyOrdersFiltered = api()->openOrdersStorage()->filter(id, Order::OrderType::Buy);
    const auto openSellOrdersFiltered = api()->openOrdersStorage()->filter(id, Order::OrderType::Sell);
    const auto &tradeHistoryFiltered = api()->tradeHistoryStorage()->filter(id);

    if ( buyOrdersFiltered.count() && sellOrdersFiltered.count() ) {
        //TODO: сделать анализ available и total
        //NOTE: Доступный баланс в BTC
        const auto &balanceBTC = api()->balancesStorage()->btcBalance();
        if ( balanceBTC.isNull() )
            return;

        //NOTE: Доступный баланс в торгуемой валюте
        const auto &balanceCurrency = api()->balancesStorage()->find(market->getCurrency());
        if ( balanceCurrency.isNull() )
            return;

        //NOTE: Торгуемая сумма из настроек
        const qreal &tradeSummBaseCurrency = market->getTradeSumm();
        //NOTE: Минимальная торговая ставка для пары
        const qreal &minimumBaseTrade = market->getMinBasePrice();
        //NOTE: торговая комиссия
        const qreal &tradeFee = market->getTradeFee() / 100; //%

        const auto &firstBuyMarketOrder = buyOrdersFiltered.first();
        if ( firstBuyMarketOrder.isNull() ) {
            LOG_ERROR("firstBuyMarketOrder.isNull()");
            return;
        }
        const auto &firstSellMarketOrder = sellOrdersFiltered.first();
        if ( firstSellMarketOrder.isNull() ) {
            LOG_ERROR("firstSellMarketOrder.isNull()");
            return;
        }
        //NOTE: поиск лучшего ордера на покупку
        const qreal &topBuyValOrder = firstBuyMarketOrder->getPrice();
        //NOTE: поиск лучшего ордера на продажу
        const qreal &topSellValOrder = firstSellMarketOrder->getPrice();

        //NOTE: если цена покупки больше цены продажи
        if ( topBuyValOrder >= topSellValOrder ) {
            LOG_WARNING(QString(QStringLiteral("Buy orders > sell orders. Can't continue trading. Waiting...")));
            return;
        }

        const qreal &difference = SettingsManager::get(SettingsManager::TradeDifference).toReal();
        //NOTE: подсчёт профита
        const qreal &profit = calculateProfit(topBuyValOrder + difference, topSellValOrder - difference, tradeFee);

        //NOTE: пересчитываем сумму валюты в BTC по актуальному курсу продажи
        const qreal &currencyTotalBalanceInBTC = balanceCurrency->getTotal() * topSellValOrder;
//        const qreal &currencyAvailableBalanceInBTC = balanceCurrency.second * topSellValOrder;
        //NOTE: если на счетё есть достаточно валюты для минимальной ставки, то продаём
        if ( currencyTotalBalanceInBTC >= minimumBaseTrade ) {
            LOG_TRACE(QString(QStringLiteral("%1 balance in BTC=%2 for sell rate=%3 is ok. Start selling..."))
                      .arg(currencyName)
                      .arg(QString::number(currencyTotalBalanceInBTC, 'f', 8))
                      .arg(QString::number(topSellValOrder, 'f', 8)));
//            if ( currencyAvailableBalanceInBTC >= tradeSummBaseCurrency ) {
            //NOTE: удаляем все ордера на покупку, если они остались открытыми
//                searchAndCancelOrders(Order::Buy);
            const Historydata &historyData = findPreviousBuyOrders(tradeHistoryFiltered);

            //NOTE: ищем шаг
            const QPair<qreal,qreal> &stepData = getStepMerchant(market->getCurrentTrendRate());
            //NOTE: считаем курс, по которому нужно докупить
            const qreal &buyInRate = calculateBuyIn(historyData.rate, historyData.fee, tradeFee, historyData.amountCurrency, stepData.second, stepData.first);

            //NOTE: ищем открытый ордер на покупку
            const auto &firstBuyOpenOrder = openBuyOrdersFiltered.first();
            bool needSetNewBuyOrder = false;
            if ( !firstBuyOpenOrder.isNull() ) {
                const quint64 &orderUintId = firstBuyOpenOrder->getOrderId().get();
                const qreal &rate = firstBuyOpenOrder->getPrice();
                const qreal &amount = firstBuyOpenOrder->getCurrencyVolume();

                //NOTE: если ордер не совпадает с тем, который должен быть на шаге, снимаем
                if ( !qFuzzyCompare(historyData.amountCurrency, amount) &&
                     !qFuzzyCompare(buyInRate, rate) ) {
                    LOG_INFO(QString(QStringLiteral("Cancelling open %1In order id=%2 rate=%3 amount=%4"))
                             .arg(api()->orderTypeToString(firstBuyOpenOrder->getType()))
                             .arg(orderUintId)
                             .arg(QString::number(rate, 'f', 8))
                             .arg(QString::number(amount, 'f', 8)));
                    api()->cancelOrder(IExchangeAPI::OrderId, firstBuyOpenOrder->getOrderId());
                    needSetNewBuyOrder = true;
                }

            } else needSetNewBuyOrder = true;
            //TODO: полную сумму в BTC надо рассчитывать от объёма закупки
            const qreal &fullBtcSumm = historyData.amountCurrency * buyInRate * (1 + tradeFee);
            if ( balanceBTC->getTotal() >= fullBtcSumm && needSetNewBuyOrder ) {
                LOG_INFO(QString(QStringLiteral("Set buyIn %1/%2 rate=%3, profit=%4, amount=%5, BTC=%6, step=%7"))
                         .arg(currencyName)
                         .arg(CONST_BTC)
                         .arg(QString::number(buyInRate, 'f', 8))
                         .arg(QString::number(stepData.second, 'f', 8))
                         .arg(QString::number(historyData.amountCurrency, 'f', 8))
                         .arg(QString::number(fullBtcSumm, 'f', 8))
                         .arg(QString::number(stepData.first, 'f', 8)));
                api()->submitOrder(market->getId(), Order::Buy, buyInRate, historyData.amountCurrency);
            }

            //NOTE: подсчёт stop-loss для нового ордера
            const qreal &stopLoss = calculateStopLoss(historyData.rate, historyData.fee, historyData.amountBtc, tradeFee, historyData.amountCurrency, market->getPercentageProfit());

            //NOTE: если есть хоть 1 ордер на продажу
            const auto &firstSellOpenOrder = openSellOrdersFiltered.first();
            if ( !firstSellOpenOrder.isNull() ) {
                //NOTE: получаем цену открытого ордера на продажу
                const qreal &openValOrder = firstSellOpenOrder->getPrice();
                const qreal &openAmount = firstSellOpenOrder->getCurrencyVolume();
                //NOTE: получем ID ордера
                const quint64 &orderId = firstSellOpenOrder->getOrderId().get();
                //NOTE: если установлен режим только продажа И открытый ордер > лучшего ордера на продажу ИЛИ выставленный объём < баланса на счету, отменяем
                if ( ( sellMode &&
                        ( (openValOrder > topSellValOrder)
                          || ( openAmount < balanceCurrency->getTotal())
                        )
                        //NOTE: ИЛИ открытый ордер > лучшего ордера на продажу И лучший ордер на продажу > stop-loss ИЛИ выставленный объём < баланса на счету, отменяем
                     ) || ( ( (openValOrder > topSellValOrder)
                              && (topSellValOrder > stopLoss) )
                            || ( openAmount < balanceCurrency->getTotal())
                          )
                   ) {
                    LOG_INFO(QString(QStringLiteral("Cancel %1 sell order id %2, open rate=%3, top sell=%4, stop-loss=%5"))
                             .arg(currencyName)
                             .arg(orderId)
                             .arg(QString::number(openValOrder, 'f', 8))
                             .arg(QString::number(topSellValOrder, 'f', 8))
                             .arg(QString::number(stopLoss, 'f', 8)));
                    api()->cancelOrder(IExchangeAPI::OrderId, firstSellOpenOrder->getOrderId());
                } else {
                    //NOTE: если разница с 2 ордером в списке больше, то снимаем
                    const auto &secondOrder = sellOrdersFiltered.at(1);
                    if ( !secondOrder.isNull() ) {
                        const qreal &value = secondOrder->getPrice();
                        if ( (value - openValOrder ) > 0.0000001 ) {
                            LOG_INFO(QString(QStringLiteral("Cancel %1 sell order id %2, open rate=%3, previous sell=%4, stop-loss=%5"))
                                     .arg(currencyName)
                                     .arg(orderId)
                                     .arg(QString::number(openValOrder, 'f', 8))
                                     .arg(QString::number(value, 'f', 8))
                                     .arg(QString::number(stopLoss, 'f', 8)));
                            api()->cancelOrder(IExchangeAPI::OrderId, secondOrder->getOrderId());
                        }
                    }
                }
            } else {
                qreal rate = topSellValOrder - difference;
                //NOTE: Если лучший ордер (который мы выставляем) < stop-loss и не установлен режим только продажа
                if ( rate < stopLoss && !sellMode ) {
                    LOG_WARNING(QString(QStringLiteral("Rate=%1 < StopLoss=%2, top sell market=%3, use stop-loss value for %4 sell order"))
                                .arg(QString::number(rate, 'f', 8))
                                .arg(QString::number(stopLoss, 'f', 8))
                                .arg(QString::number(topSellValOrder, 'f', 8))
                                .arg(currencyName));
                    rate = stopLoss;
                }
                //                const qreal &amount = tradeSummBaseCurrency / rate;
                LOG_INFO(QString(QStringLiteral("Set sell %1/%2 rate=%3, top=%4, stop-loss=%5, amount=%6"))
                         .arg(currencyName)
                         .arg(CONST_BTC)
                         .arg(QString::number(rate, 'f', 8))
                         .arg(QString::number(topSellValOrder, 'f', 8))
                         .arg(QString::number(stopLoss, 'f', 8))
                         .arg(QString::number(balanceCurrency->getTotal(), 'f', 8)));
                api()->submitOrder(market->getId(), Order::Sell, rate, balanceCurrency->getTotal());
//                    const QString &query = QString("INSERT INTO trade_journal "
//                                                   "(type, top_rate, rate, stop_loss, balance, symbol, timestamp) "
//                                                   "VALUES (%1, %2, %3, %4, %5, %6, %7)")
//                            .arg(Order::orderTypeToString(Order::Sell))
//                            .arg(QString::number(topSellValOrder, 'f', 8))
//                            .arg(QString::number(rate, 'f', 8))
//                            .arg(QString::number(stopLoss, 'f', 8))
//                            .arg(QString::number(balanceCurrency.first, 'f', 8))
//                            .arg(currencyName)
//                            .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
//                    Crud::getInstance()->executeQuery(query);
            }
//            } else {
//                LOG_WARNING(QString("Available %1 balance = %2, total = %3 Minimum available trade set is: %4. Can't set sell orders.")
//                                                     .arg(currencyName)
//                                                     .arg(QString::number(currencyAvailableBalanceInBTC, 'f', 8))
//                                                     .arg(QString::number(currencyTotalBalanceInBTC, 'f', 8))
//                                                     .arg(QString::number(tradeSummBaseCurrency, 'f', 8)));
//            }
            //NOTE: если валюты мало для торгов, то покупаем
        } else {
            //NOTE: если установлен режим только продажа
            if (sellMode) {
                searchAndCancelOrders(openBuyOrdersFiltered);
                return;
            }

            LOG_TRACE(QString(QStringLiteral("%1 balance in BTC=%2 for sell rate=%3 minimun=%4. Start buying..."))
                      .arg(currencyName)
                      .arg(QString::number(currencyTotalBalanceInBTC, 'f', 8))
                      .arg(QString::number(topSellValOrder, 'f', 8))
                      .arg(QString::number(minimumBaseTrade, 'f', 8)));
            //NOTE: удаляем все ордера на продажу, если они остались открытыми
            searchAndCancelOrders(openSellOrdersFiltered);

            //NOTE: если есть хоть 1 ордер на покупку
            const auto &firstBuyOpenOrder = openBuyOrdersFiltered.first();
            if ( !firstBuyOpenOrder.isNull() ) {
                //NOTE: получаем цену открытого ордера на покупку
                const qreal &openValOrder = firstBuyOpenOrder->getPrice();
                //NOTE: получем ID ордера
                const quint64 &orderId = firstBuyOpenOrder->getOrderId().get();

                //NOTE: если ордер перебит, отменяем
                if ( (topBuyValOrder > openValOrder) ) {
                    LOG_INFO(QString(QStringLiteral("Cancel %1 buy order id %2, open rate=%3, top buy=%4"))
                             .arg(currencyName)
                             .arg(orderId)
                             .arg(QString::number(openValOrder, 'f', 8))
                             .arg(QString::number(topBuyValOrder, 'f', 8)));
                    api()->cancelOrder(IExchangeAPI::OrderId, firstBuyOpenOrder->getOrderId());
                } else {
                    //NOTE: если профит текущего ордера упал, то снимаем ордер
                    if ( profit <= 0 && !market->getIsVolumeTrading() ) {
                        LOG_INFO(QString(QStringLiteral("Cancel %1 buy order id %2, open rate=%3, top buy=%4, top sell=%5, profit=%6"))
                                 .arg(currencyName)
                                 .arg(orderId)
                                 .arg(QString::number(openValOrder, 'f', 8))
                                 .arg(QString::number(topBuyValOrder, 'f', 8))
                                 .arg(QString::number(topSellValOrder, 'f', 8))
                                 .arg(QString::number(profit, 'f', 8)));
                        api()->cancelOrder(IExchangeAPI::OrderId, firstBuyOpenOrder->getOrderId());
                    }
                    //NOTE: если разница с 2 ордером в списке больше, то снимаем
                    const auto &secondBuyOrder = buyOrdersFiltered.at(1);
                    if ( !secondBuyOrder.isNull() ) {
                        const qreal &value = secondBuyOrder->getPrice();
                        if ( (openValOrder - value ) > 0.0000001 ) {
                            LOG_INFO(QString(QStringLiteral("Cancel %1 buy order id %2, open rate=%3, previous buy=%4"))
                                     .arg(currencyName)
                                     .arg(orderId)
                                     .arg(QString::number(openValOrder, 'f', 8))
                                     .arg(QString::number(value, 'f', 8)));
                            api()->cancelOrder(IExchangeAPI::OrderId, secondBuyOrder->getOrderId());
                        }
                    }
                }
            } else {
                //NOTE: Если профит > 0
                if ( profit > 0 || market->getIsVolumeTrading() ) {
                    //NOTE: если сумма BTC больше минимальной торговой ставки + комиссия, то выставляем ордер
                    const qreal &fullBtcSumm = tradeSummBaseCurrency * (1 + tradeFee);
                    if ( balanceBTC->getTotal() >= fullBtcSumm ) {
                        const qreal &rate = topBuyValOrder + difference;
                        const qreal &amount = tradeSummBaseCurrency / rate;
                        LOG_INFO(QString(QStringLiteral("Set buy %1/%2 rate=%3, top=%4, profit=%5, amount=%6, BTC=%7"))
                                 .arg(currencyName)
                                 .arg(CONST_BTC)
                                 .arg(DOUBLE_TO_STR(rate))
                                 .arg(DOUBLE_TO_STR(topBuyValOrder))
                                 .arg(DOUBLE_TO_STR(profit))
                                 .arg(DOUBLE_TO_STR(amount))
                                 .arg(DOUBLE_TO_STR(fullBtcSumm)));
                        api()->submitOrder(market->getId(), Order::Buy, rate, amount);
                    } else
                        LOG_WARNING(QString(QStringLiteral("Balance BTC=%1. Minimum trade=%2+%3(fee)=%4. Can't set buy orders."))
                                    .arg(DOUBLE_TO_STR(balanceBTC->getTotal()))
                                    .arg(DOUBLE_TO_STR(tradeSummBaseCurrency))
                                    .arg(DOUBLE_TO_STR(tradeSummBaseCurrency * tradeFee))
                                    .arg(DOUBLE_TO_STR(fullBtcSumm)));
                }
            }
        }
    }
}

void Spreder::dropCoin(const Market::ConstPtr &market)
{
    const auto &tradeHistoryFiltered = api()->tradeHistoryStorage()->filter(market->getId());
    const QString &currencyName = market->getCurrency()->getId().get();
    //NOTE: Доступный баланс в торгуемой валюте
    const auto &balanceCurrency = api()->balancesStorage()->find(market->getCurrency());
    const Historydata &historyData = findPreviousBuyOrders(tradeHistoryFiltered);
    //NOTE: торговая комиссия
    const qreal &tradeFee = market->getTradeFee() / 100; //%
    //NOTE: подсчёт stop-loss для нового ордера
    const qreal &stopLoss = calculateStopLoss(historyData.rate, historyData.fee, historyData.amountBtc, tradeFee, historyData.amountCurrency, market->getPercentageProfit());

    LOG_INFO(QString(QStringLiteral("Drop coin by stoploss sell %1/%2 stop-loss=%3, amount=%4"))
             .arg(currencyName)
             .arg(CONST_BTC)
             .arg(QString::number(stopLoss, 'f', 8))
             .arg(QString::number(balanceCurrency->getTotal(), 'f', 8)));
    api()->submitOrder(market->getId(), Order::Sell, stopLoss, balanceCurrency->getTotal());
}

qreal Spreder::calculateProfit(const qreal &bid, const qreal &ask, const qreal &fee) const
{
    const qreal &tradeSummBaseCurrency = SettingsManager::get(SettingsManager::TradeBaseSumm).toReal();
    const qreal &tradeFee = 1 - fee; //%
    const qreal &startBuyCurr = tradeSummBaseCurrency / bid  * tradeFee;
    const qreal &startSellCurr = startBuyCurr * ask * tradeFee;
    return startSellCurr - tradeSummBaseCurrency;
}

qreal Spreder::calculateStopLoss(const qreal &oldBid, const qreal &oldFee, const qreal &oldAmountBtc, const qreal &fee, const qreal &oldAmountCurrency, const qreal &pairProfit) const
{
    const qreal &minProfit = 1 - (100 - pairProfit ) / 100; //%
    qreal profit = oldBid * minProfit;
    if ( qFuzzyIsNull(profit) )
        profit = PRECISSION;
    const qreal &newFee = oldAmountBtc * fee;
    const qreal &stopLoss = ( (oldAmountBtc + newFee + oldFee) / oldAmountCurrency ) + profit;
    return QString::number(stopLoss,'f',8).toDouble();
}

qreal Spreder::calculateBuyIn(const qreal &oldBid, const qreal &oldFee, const qreal &fee, const qreal &oldAmountCurrency, const qreal &pairProfit, const qreal &step) const
{
    //TODO: доделать рассчёт
    // * (1 - stepData.first / 100)
    const qreal &minProfit = 1 - (100 - pairProfit ) / 100; //%
    const qreal &falling = 1 - step / 100;
    const qreal &newRate = oldBid * falling;
    qreal profit = newRate * minProfit;
    if ( qFuzzyIsNull(profit) )
        profit = PRECISSION;
    const qreal &amountBtc = newRate / oldAmountCurrency;
    const qreal &newFee = amountBtc * fee;
    const qreal &stopLoss = ( (amountBtc + newFee + oldFee) / oldAmountCurrency ) + profit;
    return QString::number(stopLoss,'f',8).toDouble();
}

Spreder::Historydata Spreder::findPreviousBuyOrders(const Order::ConstContainer &container) const noexcept
{
    Historydata ret;

    bool find = false;

    QList<qreal> ratesList;
    for (const auto &order : container) {
        if ( !order.isNull() ) {
            if (order->getType() == Order::Buy) {
                find = true;
                const qreal &oldRate = order->getPrice();
                const qreal &oldFee = order->getFee();
                const qreal &oldTotal = order->getBaseVolume();
                const qreal &oldAmount = order->getCurrencyVolume();
                ratesList << oldRate;
                ret.fee = ret.fee + oldFee;
                ret.amountBtc = ret.amountBtc + oldTotal;
                ret.amountCurrency = ret.amountCurrency + oldAmount;
            }

            if ( order->getType() == Order::Sell && find )
                break;
        }
    }

    for(const qreal &val : ratesList)
        ret.rate = ret.rate + val;

    ret.rate = ret.rate / ratesList.count();
    return ret;
}

QPair<qreal, qreal> Spreder::getStepMerchant(const qreal &trend) const
{
    Q_UNUSED(trend)

//    const auto &array = SettingsManager::get(SettingsManager::MerchantTable, SettingsManager::Merchant).toJsonArray();
    QPair<qreal,qreal> data;
//    for (const QJsonValue &v : array) {
//        const QJsonObject &obj = v.toObject();
//        const qreal &step = obj[QStringLiteral("step")].toDouble();
//        if ( trend < step ) {
//            data.first = step;
//            data.second = obj[QStringLiteral("profit")].toDouble();
//            break;
//        }
//    }
    return data;
}

void Spreder::searchAndCancelOrders(const Order::ConstContainer &container) noexcept
{
    for (const auto &order : container) {
        if ( !order.isNull() ) {
            const quint64 &orderId = order->getOrderId().get();
            const qreal &rate = order->getPrice();
            const qreal &amount = order->getCurrencyVolume();
            api()->cancelOrder(IExchangeAPI::OrderId, order->getOrderId());
            LOG_INFO(QString(QStringLiteral("Cancelling open %1 order id=%2 rate=%3 amount=%4"))
                     .arg(api()->orderTypeToString(order->getType()))
                     .arg(orderId)
                     .arg(QString::number(rate, 'f', 8))
                     .arg(QString::number(amount, 'f', 8)));
        }
    }
}

