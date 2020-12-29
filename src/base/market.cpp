#include "market.h"

Market::Market(QObject *parent)
    : QObject(parent)
    , m_config(Config())
{
}

Market::Market(const Market::Config &config)
{
    setId(config.id);
    setConfig(config);
}

//Market::Market(const Market &o) noexcept
//    : QObject (o.parent())
//    , m_tradeCurrency(Currency::Ptr::create(*o.getCurrency())) //FIXME: copy models problem
//    , m_baseCurrency(Currency::Ptr::create(*o.getBaseCurrency())) //FIXME: copy models problem
//    , m_userConfig(o.getUserConfig())
//    , m_config(o.m_config)
//{
//}

Market::Market(Market &&o) noexcept
    : QObject (o.parent())
    , m_tradeCurrency(std::move(o.m_tradeCurrency))
    , m_baseCurrency(std::move(o.m_baseCurrency))
    , m_userConfig(std::move(o.m_userConfig))
    , m_config(std::move(o.m_config))
{}

void Market::setUserConfig(const Market::UserConfig &config) noexcept
{
    bool sellMode = config.sellMode;
    bool buyMode = config.buyMode;
    bool pauseMode = config.pauseMode;
    //WARNING: снимаем копию данных, это из-за другого потока может прийти
    const qreal minTrendRate = config.minTrend;
    const qreal currentTrendRate = config.currentTrendRate;
    const QDateTime currentTrendRateTimestamp = config.lastRateUpdateTime;
    const qreal percentageProfit = config.percentageProfit;
    const qreal tradeSumm = config.tradeSumm;
    bool isVolumeTrading = config.isVolumeTrading;
    bool isTradable = config.isTradable;
    QJsonObject metaConfig = config.metaConfig;

    this->setSellMode(sellMode);
    this->setBuyMode(buyMode);
    this->setPauseMode(pauseMode);
    this->setMinTrend(minTrendRate);
    this->setCurrentTrendRate(currentTrendRate);
    this->setLastRateUpdateTime(currentTrendRateTimestamp);
    this->setPercentageProfit(percentageProfit);
    this->setIsVolumeTrading(isVolumeTrading);
    this->setIsTradable(isTradable);
    this->setMetaConfig(metaConfig);
    this->setTradeSumm(tradeSumm);
}

Currency *Market::getCurrencyPtr() const noexcept
{
    return m_tradeCurrency.constCast<Currency>().get();
}

const Currency::ConstPtr Market::getCurrency() const noexcept
{
    return m_tradeCurrency;
}

void Market::setCurrency(const Currency::ConstPtr &value) noexcept
{
    m_tradeCurrency = value;
}

Currency *Market::getBaseCurrencyPtr() const noexcept
{
    return m_baseCurrency.constCast<Currency>().get();
}

const Currency::ConstPtr Market::getBaseCurrency() const noexcept
{
    return m_baseCurrency;
}

void Market::setBaseCurrency(const Currency::ConstPtr &value) noexcept
{
    m_baseCurrency = value;
}

Market::MarketStatus Market::getStatus() const noexcept
{
    return m_config.status;
}

void Market::setStatus(const MarketStatus value) noexcept
{
    if ( m_config.status != value ) {
        m_config.status = value;
        emit statusChanged();
    }
}

QString Market::getLabel() const noexcept
{
    return QString("%1/%2").arg(m_tradeCurrency->getSymbol()).arg(m_baseCurrency->getSymbol());
}

const qreal &Market::getVolumeStepSize() const noexcept
{
    return m_config.volumeStepSize;
}

void Market::setVolumeStepSize(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.volumeStepSize, value)
         && !qFuzzyIsNull(value) )
        m_config.volumeStepSize = value;
}

const qreal &Market::getTradeSumm() const noexcept
{
    return m_userConfig.tradeSumm;
}

void Market::setTradeSumm(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_userConfig.tradeSumm, value)
         && !qFuzzyIsNull(value) ) {
        m_userConfig.tradeSumm = value;
        emit tradeSummChanged();
    }
}

const QString &Market::getStatusMessage() const noexcept
{
    return m_config.statusMessage;
}

void Market::setStatusMessage(const QString &value) noexcept
{
    if ( m_config.statusMessage != value
         && !value.isEmpty()) {
        m_config.statusMessage = value;
        emit statusMessageChanged();
    }
}

const QDateTime &Market::getLastRateUpdateTime() const noexcept
{
    return m_userConfig.lastRateUpdateTime;
}

void Market::setLastRateUpdateTime(const QDateTime &value) noexcept
{
    if ( m_userConfig.lastRateUpdateTime != value
         && !value.isNull()) {
        m_userConfig.lastRateUpdateTime = value;
        emit lastRateUpdateTimeChanged();
    }
}

const qreal &Market::getAskPrice() const noexcept
{
    return m_config.askPrice;
}

void Market::setAskPrice(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.askPrice, value)
         && !qFuzzyIsNull(value) ) {
        m_config.askPrice = value;
        emit askPriceChanged();
    }
}

void Market::setAskPrice(qreal &&value) noexcept
{
    if ( !qFuzzyCompare(m_config.askPrice, value)
         && !qFuzzyIsNull(value) ) {
        m_config.askPrice = std::move(value);
        emit askPriceChanged();
    }
}

const qreal &Market::getBidPrice() const noexcept
{
    return m_config.bidPrice;
}

void Market::setBidPrice(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.bidPrice, value)
         && !qFuzzyIsNull(value) ) {
        m_config.bidPrice = value;
        emit bidPriceChanged();
    }
}

const qreal &Market::getLow() const noexcept
{
    return m_config.low;
}

void Market::setLow(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.low, value)
         && !qFuzzyIsNull(value) ) {
        m_config.low = value;
        emit lowChanged();
    }
}

const qreal &Market::getHigh() const noexcept
{
    return m_config.high;
}

void Market::setHigh(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.high, value)
         && !qFuzzyIsNull(value) ) {
        m_config.high = value;
        emit highChanged();
    }
}

const qreal &Market::getVolume() const noexcept
{
    return m_config.volume;
}

void Market::setVolume(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.volume, value)
         && !qFuzzyIsNull(value) ) {
        m_config.volume = value;
        emit volumeChanged();
    }
}

const qreal &Market::getLastPrice() const noexcept
{
    return m_config.lastPrice;
}

void Market::setLastPrice(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.lastPrice, value)
         && !qFuzzyIsNull(value) ) {
        m_config.lastPrice = value;
        emit lastPriceChanged();
    }
}

const qreal &Market::getChange() const noexcept
{
    return m_config.change;
}

void Market::setChange(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.change, value)
         && !qFuzzyIsNull(value) ) {
        m_config.change = value;
        emit changeChanged();
    }
}

const qreal &Market::getOpen() const noexcept
{
    return m_config.open;
}

void Market::setOpen(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.open, value)
         && !qFuzzyIsNull(value) ) {
        m_config.open = value;
        emit openChanged();
    }
}

const qreal &Market::getClose() const noexcept
{
    return m_config.close;
}

void Market::setClose(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.close, value)
         && !qFuzzyIsNull(value) ) {
        m_config.close = value;
        emit closeChanged();
    }
}

const qreal &Market::getBaseVolume() const noexcept
{
    return m_config.baseVolume;
}

void Market::setBaseVolume(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.baseVolume, value)
         && !qFuzzyIsNull(value) ) {
        m_config.baseVolume = value;
        emit baseVolumeChanged();
    }
}

const qreal &Market::getSpread() const noexcept
{
    return m_config.spread;
}

void Market::setSpread(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.spread, value)
         && !qFuzzyIsNull(value) ) {
        m_config.spread = value;
        emit spreadChanged();
    }
}

const qreal &Market::getTradeFee() const noexcept
{
    return m_config.tradeFee;
}

void Market::setTradeFee(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.tradeFee, value)
         && !qFuzzyIsNull(value) ) {
        m_config.tradeFee = value;
        emit tradeFeeChanged();
    }
}

const qreal &Market::getMinBasePrice() const noexcept
{
    return m_config.minBaseTradePrice;
}

void Market::setMinBasePrice(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.minBaseTradePrice, value)
         && !qFuzzyIsNull(value) ) {
        m_config.minBaseTradePrice = value;
        emit minBaseTradePriceChanged();
    }
}

const qreal &Market::getMaxBasePrice() const noexcept
{
    return m_config.maxBaseTradePrice;
}

void Market::setMaxBasePrice(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.maxBaseTradePrice, value)
         && !qFuzzyIsNull(value) ) {
        m_config.maxBaseTradePrice = value;
        emit maxBaseTradePriceChanged();
    }
}

const qreal &Market::getCurrentTrendRate() const noexcept
{
    return m_userConfig.currentTrendRate;
}

void Market::setCurrentTrendRate(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_userConfig.currentTrendRate, value) ) {
        m_userConfig.currentTrendRate = value;
        emit currentTrendRateChanged();
    }
}

const qreal &Market::getPercentageProfit() const noexcept
{
    return m_userConfig.percentageProfit;
}

 qreal Market::getPercentageProfit100thPercent() const noexcept
{
    return m_userConfig.percentageProfit / 100;
}

void Market::setPercentageProfit(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_userConfig.percentageProfit, value) ) {
        m_userConfig.percentageProfit = value;
        emit percentageProfitChanged();
    }
}

const qreal &Market::getMinTrend() const noexcept
{
    return m_userConfig.minTrend;
}

void Market::setMinTrend(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_userConfig.minTrend, value) ) {
        m_userConfig.minTrend = value;
        emit minTrendChanged();
    }
}

const Market::MarketId &Market::getId() const noexcept
{
    return m_config.id;
}

void Market::setId(const MarketId &value) noexcept
{
    if ( m_config.id != value ) {
        m_config.id = value;
        emit idChanged();
    }
}

bool Market::getIsTradable() const noexcept
{
    return m_userConfig.isTradable;
}

void Market::setIsTradable(const bool value) noexcept
{
    if ( m_userConfig.isTradable != value ) {
        m_userConfig.isTradable = value;
        emit isTradableChanged();
    }
}

bool Market::getIsVolumeTrading() const noexcept
{
    return m_userConfig.isVolumeTrading;
}

void Market::setIsVolumeTrading(const bool value) noexcept
{
    if ( m_userConfig.isVolumeTrading != value ) {
        m_userConfig.isVolumeTrading = value;
        emit isVolumeTradingChanged();
    }
}

bool Market::getPauseMode() const noexcept
{
    return m_userConfig.pauseMode;
}

void Market::setPauseMode(const bool value) noexcept
{
    if ( m_userConfig.pauseMode != value) {
        m_userConfig.pauseMode = value;
        emit pauseModeChanged();
    }
}

bool Market::getSellMode() const noexcept
{
    return m_userConfig.sellMode;
}

void Market::setSellMode(const bool value) noexcept
{
    if ( m_userConfig.sellMode != value ) {
        m_userConfig.sellMode = value;
        emit sellModeChanged();
    }
}

bool Market::getBuyMode() const noexcept
{
    return m_userConfig.buyMode;
}

void Market::setBuyMode(const bool value) noexcept
{
    if ( m_userConfig.buyMode != value ) {
        m_userConfig.buyMode = value;
        emit buyModeChanged();
    }
}

const Market::UserConfig &Market::getUserConfig() const noexcept
{
    return m_userConfig;
}

void Market::setConfig(const Market::Config &config) noexcept
{
    setStatusMessage(config.statusMessage);
    setAskPrice(config.askPrice);
    setBidPrice(config.bidPrice);
    setLow(config.low);
    setHigh(config.high);
    setVolume(config.volume);
    setLastPrice(config.lastPrice);
    setChange(config.change);
    setOpen(config.open);
    setClose(config.close);
    setBaseVolume(config.baseVolume);
    setSpread(config.spread);
    setMinBasePrice(config.minBaseTradePrice);
    setMaxBasePrice(config.maxBaseTradePrice);
    setPriceTickSize(config.priceTickSize);
    setVolumeStepSize(config.volumeStepSize);
    setMinBaseVolume(config.minBaseVolume);
    setMaxBaseVolume(config.maxBaseVolume);
    setMinNotional(config.minNotional);
    setTradeFee(config.tradeFee);
    setStatus(config.status);
}

const QJsonObject &Market::getMetaConfig() const noexcept
{
    return m_userConfig.metaConfig;
}

void Market::setMetaConfig(const QJsonObject &data) noexcept
{
    if ( m_userConfig.metaConfig != data ) {
        m_userConfig.metaConfig = data;
        emit metaConfigChanged();
    }
}

const qreal &Market::getPriceTickSize() const noexcept
{
    return m_config.priceTickSize;
}

void Market::setPriceTickSize(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.priceTickSize, value)
         && !qFuzzyIsNull(value) ) {
        m_config.priceTickSize = value;
        emit priceTickChanged();
    }
}

const qreal &Market::getMinBaseVolume() const noexcept
{
    return m_config.minBaseVolume;
}

void Market::setMinBaseVolume(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.minBaseVolume, value)
         && !qFuzzyIsNull(value) ) {
        m_config.minBaseVolume = value;
        emit minBaseVolumeChanged();
    }
}

const qreal &Market::getMaxBaseVolume() const noexcept
{
    return m_config.maxBaseVolume;
}

void Market::setMaxBaseVolume(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.maxBaseVolume, value)
         && !qFuzzyIsNull(value) ) {
        m_config.maxBaseVolume = value;
        emit maxBaseVolumeChanged();
    }
}

const qreal &Market::getMinNotional() const noexcept
{
    return m_config.minNotional;
}

void Market::setMinNotional(const qreal &value) noexcept
{
    if ( !qFuzzyCompare(m_config.minNotional, value)
         && !qFuzzyIsNull(value) ) {
        m_config.minNotional = value;
        emit minNotionalChanged();
    }
}

bool Market::UserConfig::operator==(const Market::UserConfig &o) const noexcept {
    bool ret =
            (
                (lastRateUpdateTime == o.lastRateUpdateTime) &&
                qFuzzyCompare(percentageProfit, o.percentageProfit) &&
                qFuzzyCompare(minTrend, o.minTrend) &&
                qFuzzyCompare(currentTrendRate, o.currentTrendRate) &&
                (isTradable == o.isTradable) &&
                (isVolumeTrading == o.isVolumeTrading) &&
                (pauseMode == o.pauseMode) &&
                (sellMode == o.sellMode) &&
                (buyMode == o.buyMode) &&
                (metaConfig == o.metaConfig) &&
                (qFuzzyCompare(tradeSumm, o.tradeSumm))
                );
    return ret;
}
