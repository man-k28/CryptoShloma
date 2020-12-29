#include "step.h"
#include <Logger.h>
#include <algo/hanukkah.h>
#include <api/i_exchange_api.h>
#include <storage/balances_storage.h>
#include <storage/market_orders_storage.h>
#include <storage/open_orders_storage.h>
#include <storage/trade_history_storage.h>
#include <common/utils.h>

Step::Step(const qreal &declinePerc,
                 const qreal &growth,
                 Hanukkah * const parentAlgo,
                 const Market::ConstPtr &market,
                 bool buyBlocked,
                 bool sellBlocked) noexcept
    : IStep(declinePerc, growth, parentAlgo, market, buyBlocked, sellBlocked)
{
}

void Step::executeSellLogic() noexcept
{
    if ( m_market.isNull() ) {
        LOG_DEBUG(QStringLiteral("Market is null"));
        return;
    }

    const auto &api = m_parentAlgo->api();
    //NOTE: торговая комиссия
    const qreal &tradeFee = m_market->getTradeFee(); //%
    //NOTE: Доступный баланс в торгуемой валюте
    const auto &balanceCurrency = api->balancesStorage()->find(m_market->getCurrency());
    if ( balanceCurrency.isNull() ) {
        LOG_DEBUG(QString(QStringLiteral("Balance of %1 is null")).arg(m_market->getCurrency()->getSymbol()));
        return;
    }

    auto metaConfig = m_market->getMetaConfig();
    const qint32 execSteps = metaConfig.value(QStringLiteral("execSteps")).toInt();
    const qint32 maxSteps = metaConfig.value(QStringLiteral("maxSteps")).toInt();
    const QDateTime &cycleTimeStart = QDateTime::fromString(metaConfig.value(QStringLiteral("cycleTimeStart")).toString(), Qt::ISODate);

    const auto openSellOrdersFiltered = api->openOrdersStorage()->filter(m_market->getId(), Order::OrderType::Sell);

    //NOTE: проверяем, есть ли открытый ордер
    if ( !openSellOrdersFiltered.count() ) {
        const auto &tradeHistoryFiltered = api->tradeHistoryStorage()->filter(m_market->getId());
        if ( !tradeHistoryFiltered.count())
            return;
        const auto &averageData = calculateAverate(tradeHistoryFiltered, cycleTimeStart);
        const qreal &currencyVolume = balanceCurrency->getTotal();
        const qreal &currencyVolumeAvailable = balanceCurrency->getAvailable();
        //NOTE: подсчёт stop-loss для нового ордера
        const qreal &stopLoss = calculateStopLoss(averageData.price,
                                                  averageData.fee,
                                                  averageData.baseVolume,
                                                  tradeFee,
                                                  averageData.volume,
                                                  m_market->getPercentageProfit100thPercent());

        const qreal &fullBaseVolume = currencyVolumeAvailable * stopLoss;

        //NOTE: если баланс пуст, считаем, что продали
        if ( ( qFuzzyIsNull(currencyVolume) && qFuzzyIsNull(currencyVolumeAvailable) )
             || (m_market->getVolumeStepSize() > currencyVolume)
             || (m_market->getMinNotional() > fullBaseVolume )) {
            LOG_INFO(QString("%1 was sell").arg(m_market->getLabel()));
            unlock();
            return;
        }

        if ( currencyVolume > currencyVolumeAvailable && !qFuzzyCompare(currencyVolume, currencyVolumeAvailable) ) {
            LOG_WARNING(QString("Insufficient funds of %1 in access: total:%2 available:%3")
                            .arg(m_market->getLabel())
                            .arg(DOUBLE_TO_STR(currencyVolume))
                            .arg(DOUBLE_TO_STR(currencyVolumeAvailable)));
            return;
        }

        //NOTE: либо размещаем новый ордер на продажу на весь баланс по средневзвешанному курсу (если есть мусор, то его курс не учитывается, просто кладём в кучу)

        api->submitOrder(m_market->getId(), Order::Sell, stopLoss, fullBaseVolume, currencyVolumeAvailable);
    } else {
        const auto &firstSellOpenOrder = openSellOrdersFiltered.first();
        const qreal &roundingBalance = CryptoShloma::Utils::rounding(m_market->getVolumeStepSize(), balanceCurrency->getTotal());

        if (firstSellOpenOrder->getCurrencyVolume() < roundingBalance && !qFuzzyCompare(firstSellOpenOrder->getCurrencyVolume(), roundingBalance)) {//FIXME: double compare
            LOG_INFO(QString(QStringLiteral("Cancel %1 sell order id %2, open volume=%3, balance volume=%4"))
                         .arg(m_market->getLabel())
                         .arg(firstSellOpenOrder->getOrderId().get())
                         .arg(DOUBLE_TO_STR(firstSellOpenOrder->getCurrencyVolume()))
                         .arg(DOUBLE_TO_STR(balanceCurrency->getTotal())));
            api->cancelOrder(IExchangeAPI::OrderId, firstSellOpenOrder->getOrderId(), m_market->getId());
            return;
        }

        //NOTE: если мы на последнем шаге, то не снимаем, будет висеть вечно
        if ( execSteps >= maxSteps )
            return;

        //NOTE: смотрим, если рейтинг упал за пределы следущего шага, то просто отменяем ордер и блочим шаг
        if ( !getNext().isNull() && m_market->getCurrentTrendRate() < -getNext()->getGrowth() ) {//FIXME: double compare
            //NOTE: иначе снимаем ордер с продажи и блочим шаг
            LOG_INFO(QString(QStringLiteral("Cancel %1 sell order id %2, open price=%3"))
                         .arg(m_market->getLabel())
                         .arg(firstSellOpenOrder->getOrderId().get())
                         .arg(DOUBLE_TO_STR(firstSellOpenOrder->getPrice())));
            api->cancelOrder(IExchangeAPI::OrderId, firstSellOpenOrder->getOrderId(), m_market->getId());
            LOG_INFO(QString(QStringLiteral("Step (decline: %1, growth: %2) %3 of selling is closed by trend rate"))
                     .arg(getDeclinePerc())
                     .arg(getGrowth())
                     .arg(m_market->getLabel()));
            setSellBlocked(true);
            updateMarketMetaConfig(false);
        }
    }
}

qreal Step::calculatePrevVolume(const Order::ConstContainer &container) noexcept
{
    qreal ret = 0.;

    for ( auto order = ++container.begin(); order != container.end(); ++order ) {
        if ((*order)->getType() == Order::Buy) {
            ret += (*order)->getCurrencyVolume();
        }

        if ( (*order)->getType() == Order::Sell )
            break;
    }

    ret = std::floor(ret * std::pow(10,8)) * PRECISSION;
    return ret;
}

void Step::executeBuyLogic() noexcept
{
    if ( m_market.isNull() ) {
        LOG_DEBUG(QStringLiteral("Market is null"));
        return;
    }
    const auto &api = m_parentAlgo->api();
    //NOTE: Минимальная торговая ставка для пары
    const qreal &minimumBaseTrade = m_market->getMinBasePrice();
    //NOTE: торговая комиссия
    const qreal &tradeFee = m_market->getTradeFee(); //%
    //NOTE: Доступный баланс в базовой валюте
    const auto &balanceBase = api->balancesStorage()->find(m_market->getBaseCurrency());
    if ( balanceBase.isNull() )
        return;

    //NOTE: Доступный баланс в торгуемой валюте
    const auto &balanceCurrency = api->balancesStorage()->find(m_market->getCurrency());
    if ( balanceCurrency.isNull() ) {
        return;
    }
    //NOTE: ордера sell по рынку
    const auto &sellOrdersFiltered = api->marketOrdersStorage()->filter(m_market->getId(), Order::OrderType::Sell);
    if ( !sellOrdersFiltered.size() ) {
        return;
    }
    //NOTE: топовый ордер на продажу по рынку
    const auto &firstSellMarketOrder = sellOrdersFiltered.first();
    //NOTE: цена лучшего ордера на продажу
    const qreal &topSellPriceOrder = firstSellMarketOrder->getPrice();

    const auto &buyOrdersFiltered = api->marketOrdersStorage()->filter(m_market->getId(), Order::OrderType::Buy);
    if ( !buyOrdersFiltered.size() ) {
        return;
    }
    //NOTE: берём лучший ордер на покупку
    const qreal &topBuyPriceOrder = buyOrdersFiltered.first()->getPrice();

    //NOTE: если цена покупки больше цены продажи (перестраховываемся от кривых данных по рынку)
    if ( topBuyPriceOrder >= topSellPriceOrder ) {
        LOG_WARNING(QString(QStringLiteral("Buy orders > sell orders. Can't continue trading. Waiting...")));
        return;
    }

    auto metaConfig = m_market->getMetaConfig();
    auto array = metaConfig.value(QStringLiteral("hannukahTable")).toArray();
    const qint32 execSteps = metaConfig.value(QStringLiteral("execSteps")).toInt();
    const QDateTime &cycleTimeStart = QDateTime::fromString(metaConfig.value(QStringLiteral("cycleTimeStart")).toString(), Qt::ISODate);

    //NOTE: смотрим, если рейтинг упал за пределы следущего шага, то блочим шаг
    if ( !getNext().isNull() && m_market->getCurrentTrendRate() <= -getNext()->getDeclinePerc() ) {//FIXME: double compare
        setSellBlocked(true);
        setBuyBlocked(true);
        updateMarketMetaConfig(false);
        LOG_INFO(QString(QStringLiteral("Current trend of %1=%2").arg(m_market->getLabel()).arg(m_market->getCurrentTrendRate())));
        return;
    }
    //NOTE: если произошёл отсток, то докупаем
    if ( m_market->getCurrentTrendRate() >= -this->getGrowth() ) {
        //NOTE: пересчитываем объём валюты на балансе в базовой валюте по топовому курсу продажи
        const qreal &currencyTotalBalance = balanceCurrency->getTotal();
        const qreal &currencyTotalBaseBalance = balanceCurrency->getTotal() * topSellPriceOrder;

        if ( qFuzzyIsNull(currencyTotalBaseBalance) ) {
            LOG_ERROR(QString("Balance %1 is 0!!").arg(m_market->getCurrency()->getSymbol()));
            return;
        }

        const auto &tradeHistoryFiltered = api->tradeHistoryStorage()->filter(m_market->getId());
        const auto &averageData = calculateAverate(tradeHistoryFiltered, cycleTimeStart);

        //NOTE: если кол-во закрытых ордеров больше выполненных шагов, то закрываем покупку ( -1 нужен для учёта первой покупки)
        //NOTE: если на счетё есть достаточно средств для ставки (x2 баланса и (-5%)) и хватает для минимальной ставки, то закрываем шаг, уходим на продажу
        //BUG: если ордеров будет несколько по частичному закрытию, то логика не сработает правильно, пока ориентируемся только на Binance FOK, в других случаях работать не будет, в api склеиваем ордера
        if ( averageData.ordersCount > execSteps ) {
            if  ( ( currencyTotalBalance >= ( (calculatePrevVolume(tradeHistoryFiltered) * 2) * 0.95) ) &&
                ( currencyTotalBaseBalance >= minimumBaseTrade ) ) {
                LOG_INFO(QString(QStringLiteral("Step (decline: %1, growth: %2) %3 of buying is closed by completed"))
                         .arg(getDeclinePerc())
                         .arg(getGrowth())
                         .arg(m_market->getLabel()));
                setBuyBlocked(true);
                updateMarketMetaConfig(true);
            }
        }
        //NOTE: если средств мало для торгов, то покупаем
        else {
            //NOTE: получаем объём лучшего ордера в базовой валюте на продажу по рынку
            const qreal &firstSellMarketOrderVolume = firstSellMarketOrder->getBaseVolume();
            //NOTE: если баланса базовой валюты хватает для покупки (купленного объёма х2 + комиссия), то выставляем ордер
            const qreal &fullBaseVolume = currencyTotalBaseBalance * (1 + tradeFee);

            //NOTE: если объёма хватает для выкупа из этого ордера сразу, то ставим ордер
            if ( (firstSellMarketOrderVolume >= fullBaseVolume ) &&
                (balanceBase->getTotal() >= fullBaseVolume )
                ) {

                const auto openBuyOrdersFiltered = api->openOrdersStorage()->filter(m_market->getId(), Order::OrderType::Buy);

                //NOTE: проверяем, есть ли открытый ордер
                if ( !openBuyOrdersFiltered.count() )
                    api->submitOrder(m_market->getId(), Order::Buy, topSellPriceOrder, fullBaseVolume, currencyTotalBalance);
            } else
                LOG_INFO(QString("The top order %1 volume=%2 is not equivalent").arg(m_market->getLabel()).arg(DOUBLE_TO_STR(firstSellMarketOrderVolume)));
        }
    }
}
