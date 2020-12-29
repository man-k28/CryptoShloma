#include "zero_step.h"
#include <algo/hanukkah.h>
#include <api/i_exchange_api.h>
#include <storage/balances_storage.h>
#include <storage/market_orders_storage.h>
#include <storage/open_orders_storage.h>
#include <storage/trade_history_storage.h>
#include <Logger.h>
#include <common/utils.h>

ZeroStep::ZeroStep(const qreal &declinePerc,
                         const qreal &growth,
                         Hanukkah * const parentAlgo,
                         const Market::ConstPtr &market,
                         bool buyBlocked,
                         bool sellBlocked) noexcept
    : IStep(declinePerc, growth, parentAlgo, market, buyBlocked, sellBlocked)
{
}

void ZeroStep::executeSellLogic() noexcept
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
    auto array = metaConfig.value(QStringLiteral("hannukahTable")).toArray();
    const QDateTime &cycleTimeStart = QDateTime::fromString(metaConfig.value(QStringLiteral("cycleTimeStart")).toString(), Qt::ISODate);

    if ( array.count() > 2 ) { //TODO: надо убрать шаг 0 и в таблице не должно быть меньше 2-х шагов пока что
        const qreal &step = array.at(1).toObject().value(QStringLiteral("step")).toDouble();
        if ( qFuzzyIsNull(step) ) {
            LOG_DEBUG("Не прочитали таблицу, шаг 1 пустой");
            return;
        }
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

            const qreal &fullBaseSumm = currencyVolumeAvailable * stopLoss;

            //NOTE: если баланс пуст, считаем, что продали
            if ( ( qFuzzyIsNull(currencyVolume) && qFuzzyIsNull(currencyVolumeAvailable) )
                 || (m_market->getVolumeStepSize() > currencyVolume)
                 || (m_market->getMinNotional() > fullBaseSumm )) {
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


            api->submitOrder(m_market->getId(), Order::Sell, stopLoss, fullBaseSumm, currencyVolumeAvailable);
        } else {
            const auto &firstSellOpenOrder = openSellOrdersFiltered.first();
            const qreal &roundingBalance = CryptoShloma::Utils::rounding(m_market->getVolumeStepSize(), balanceCurrency->getTotal());
            if (firstSellOpenOrder->getCurrencyVolume() < roundingBalance && !qFuzzyCompare(firstSellOpenOrder->getCurrencyVolume(), roundingBalance) ) {//FIXME: double compare
                LOG_INFO(QString(QStringLiteral("Cancel %1 sell order id %2, open volume=%3, balance volume=%4"))
                             .arg(m_market->getLabel())
                             .arg(firstSellOpenOrder->getOrderId().get())
                             .arg(DOUBLE_TO_STR(balanceCurrency->getTotal()))
                             .arg(DOUBLE_TO_STR(firstSellOpenOrder->getCurrencyVolume())));
                api->cancelOrder(IExchangeAPI::OrderId, firstSellOpenOrder->getOrderId(), m_market->getId());
                return;
            }

            //NOTE: смотрим, если рейтинг упал за пределы первого шага, то просто отменяем ордер и блочим шаг
            if ( m_market->getCurrentTrendRate() < -step ) {//FIXME: double compare
                //NOTE: иначе снимаем ордер с продажи и блочим шаг
                LOG_INFO(QString(QStringLiteral("Cancel %1 sell order id %2, open price=%3"))
                             .arg(m_market->getLabel())
                             .arg(firstSellOpenOrder->getOrderId().get())
                             .arg(DOUBLE_TO_STR(firstSellOpenOrder->getPrice())));
                api->cancelOrder(IExchangeAPI::OrderId, firstSellOpenOrder->getOrderId(), m_market->getId());
                LOG_INFO(QString(QStringLiteral("Step 0 %1 of selling is closed")).arg(m_market->getLabel()));
                setSellBlocked(true);
                updateMarketMetaConfig(false);
            }
        }
    }
    else {
        LOG_DEBUG(QObject::tr("По неведомой причине не загрузилась таблица настроек из пары"));
    }
}

void ZeroStep::executeBuyLogic() noexcept
{
    if ( m_market.isNull() ) {
        LOG_DEBUG(QStringLiteral("Market is null"));
        return;
    }

    //NOTE: если стоим в паузе, то не покупаем на 0 шаге
    if ( m_market->getPauseMode() )
        return;

    const auto &api = m_parentAlgo->api();
    //NOTE: Торгуемая сумма из настроек пары
    const qreal &tradeSummBaseCurrency = m_market->getTradeSumm();
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

    //NOTE: пересчитываем объём валюты на балансе в BTC по топовому курсу продажи
    const qreal &currencyTotalBalanceInBTC = balanceCurrency->getTotal() * topSellPriceOrder;

    //NOTE: если на счетё есть достаточно средств для ставки и хватает купленного объёма (-5%) из настроек, то закрываем шаг, уходим на продажу
    if ( currencyTotalBalanceInBTC >= (tradeSummBaseCurrency * 0.95) &&
         currencyTotalBalanceInBTC >= minimumBaseTrade ) {
        const auto &tradeHistoryFiltered = api->tradeHistoryStorage()->filter(m_market->getId());
        if ( !tradeHistoryFiltered.count())
            return;
        const auto &buyHistoryOrder = tradeHistoryFiltered.first();
        if ( buyHistoryOrder->getType() != Order::Buy)
            return;

        LOG_INFO(QString(QStringLiteral("Step 0 %1 of buying is closed")).arg(m_market->getLabel()));
        setBuyBlocked(true);
        updateMarketMetaConfig(true, buyHistoryOrder->getUpdatedTimeStamp());
    }
    //NOTE: если средств мало для торгов, то покупаем
    else {
        //NOTE: получаем объём лучшего ордера в BTC на продажу по рынку
        const qreal &firstSellMarketOrderVolume = firstSellMarketOrder->getBaseVolume();
        //NOTE: если баланса BTC хватает для покупки (минимальной торговой ставки + комиссия), то выставляем ордер
        const qreal &fullBtcSumm = tradeSummBaseCurrency * (1 + tradeFee);

        //NOTE: если объёма хватает для выкупа из этого ордера сразу, то ставим ордер
        if ( (firstSellMarketOrderVolume >= fullBtcSumm ) &&
            (balanceBase->getTotal() >= fullBtcSumm )
           ) {
                const auto openSellOrdersFiltered = api->openOrdersStorage()->filter(m_market->getId(), Order::OrderType::Buy);
                //NOTE: проверяем, есть ли открытый ордер
                if ( !openSellOrdersFiltered.count() ) {
                    const qreal &currencyVolume = tradeSummBaseCurrency / topSellPriceOrder;
                    api->submitOrder(m_market->getId(), Order::Buy, topSellPriceOrder, fullBtcSumm, currencyVolume);
                }
        } else {
            LOG_INFO(QString("The top order %1 volume=%2 is not equivalent").arg(m_market->getLabel()).arg(DOUBLE_TO_STR(firstSellMarketOrderVolume)));
        }
    }
}
