#ifndef MARKET_H
#define MARKET_H

#include <QDateTime>
#include <QJsonObject>
#include "currency.h"
class Market;

#pragma pack (push, 1)
class Market final : public QObject, public Boilerplate::BaseEntry<Market, QString>
{
    Q_OBJECT
    Q_PROPERTY(Currency *currency READ getCurrencyPtr CONSTANT)
    Q_PROPERTY(Currency *baseCurrency READ getBaseCurrencyPtr CONSTANT)
    Q_PROPERTY(MarketStatus status READ getStatus NOTIFY statusChanged) //TODO зарегистрировать в qml
    Q_PROPERTY(QString label READ getLabel NOTIFY idChanged)
    Q_PROPERTY(QString statusMessage READ getStatusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QDateTime lastRateUpdateTime READ getLastRateUpdateTime NOTIFY lastRateUpdateTimeChanged)
    Q_PROPERTY(qreal askPrice READ getAskPrice NOTIFY askPriceChanged)
    Q_PROPERTY(qreal bidPrice READ getBidPrice NOTIFY bidPriceChanged)
    Q_PROPERTY(qreal low READ getLow NOTIFY lowChanged)
    Q_PROPERTY(qreal high READ getHigh NOTIFY highChanged)
    Q_PROPERTY(qreal volume READ getVolume NOTIFY volumeChanged)
    Q_PROPERTY(qreal lastPrice READ getLastPrice NOTIFY lastPriceChanged)
    Q_PROPERTY(qreal change READ getChange NOTIFY changeChanged)
    Q_PROPERTY(qreal open READ getOpen NOTIFY openChanged)
    Q_PROPERTY(qreal close READ getClose NOTIFY closeChanged)
    Q_PROPERTY(qreal baseVolume READ getBaseVolume NOTIFY baseVolumeChanged)
    Q_PROPERTY(qreal spread READ getSpread NOTIFY spreadChanged)
    Q_PROPERTY(qreal tradeFee READ getTradeFee NOTIFY tradeFeeChanged)
    Q_PROPERTY(qreal minimumBaseTrade READ getMinBasePrice NOTIFY minBaseTradePriceChanged)
    Q_PROPERTY(qreal maximumBaseTrade READ getMaxBasePrice NOTIFY maxBaseTradePriceChanged)
    Q_PROPERTY(qreal currentTrendRate READ getCurrentTrendRate NOTIFY currentTrendRateChanged)
    Q_PROPERTY(qreal percentageProfit READ getPercentageProfit WRITE setPercentageProfit NOTIFY percentageProfitChanged)
    Q_PROPERTY(qreal minTrend READ getMinTrend NOTIFY minTrendChanged)
    Q_PROPERTY(Market::MarketId id READ getId NOTIFY idChanged)
    Q_PROPERTY(bool isTradable READ getIsTradable WRITE setIsTradable NOTIFY isTradableChanged)
    Q_PROPERTY(bool isVolumeTrading READ getIsVolumeTrading WRITE setIsVolumeTrading NOTIFY isVolumeTradingChanged)
    Q_PROPERTY(bool pauseMode READ getPauseMode WRITE setPauseMode NOTIFY pauseModeChanged)
    Q_PROPERTY(bool sellMode READ getSellMode WRITE setSellMode NOTIFY sellModeChanged)
    Q_PROPERTY(bool buyMode READ getBuyMode WRITE setBuyMode NOTIFY buyModeChanged)
    Q_PROPERTY(QJsonObject metaConfig READ getMetaConfig WRITE setMetaConfig NOTIFY metaConfigChanged)
    Q_PROPERTY(Market::UserConfig userConfig READ getUserConfig)
    Q_PROPERTY(qreal tradeSumm READ getTradeSumm WRITE setTradeSumm NOTIFY tradeSummChanged)
public:
    enum MarketStatus {
        Ok = 1,
        Closed,
        Paused
    };
    Q_ENUM(MarketStatus)

    using MarketId = Id;

    struct UserConfig
    {
        bool operator==(const UserConfig &o) const noexcept;
        QDateTime   lastRateUpdateTime{};
        qreal       percentageProfit{0.};
        qreal       minTrend{0.};
        qreal       currentTrendRate{0.};
        bool        isTradable{false};
        bool        isVolumeTrading{false};
        bool        pauseMode{false};
        bool        sellMode{false};
        bool        buyMode{false};
        QJsonObject metaConfig{};
        qreal       tradeSumm{0.};
    };

    struct Config
    {
        QString             statusMessage{};
        qreal               askPrice{0.};
        qreal               bidPrice{0.};
        qreal               low{0.};
        qreal               high{0.};
        qreal               volume{0.};
        qreal               lastPrice{0.};
        qreal               change{0.}; //deprecated
        qreal               open{0.};
        qreal               close{0.};
        qreal               baseVolume{0.};
        qreal               spread{0.};
        qreal               minBaseTradePrice{0.};
        qreal               maxBaseTradePrice{0.};
        qreal               priceTickSize{0.};
        qreal               volumeStepSize{0.};
        qreal               minBaseVolume{0.};
        qreal               maxBaseVolume{0.};
        qreal               minNotional{0.};
        qreal               tradeFee{0.};
        MarketStatus        status{Ok};
        MarketId            id{};
    };

public:
    explicit Market(QObject *parent = nullptr);
    explicit Market(const Config &config);
//    Market(const Market &o) noexcept;
    Market(Market &&o) noexcept;

    const Currency::ConstPtr getCurrency() const noexcept;
    void setCurrency(const Currency::ConstPtr &value) noexcept;

    const Currency::ConstPtr getBaseCurrency() const noexcept;
    void setBaseCurrency(const Currency::ConstPtr &value) noexcept;

    MarketStatus getStatus() const noexcept;
    void setStatus(const MarketStatus value) noexcept;

    const QString &getStatusMessage() const noexcept;
    void setStatusMessage(const QString &value) noexcept;

    const QDateTime &getLastRateUpdateTime() const noexcept;
    void setLastRateUpdateTime(const QDateTime &value) noexcept;

    const qreal &getAskPrice() const noexcept;
    void setAskPrice(const qreal &value) noexcept;
    void setAskPrice(qreal &&value) noexcept;

    const qreal &getBidPrice() const noexcept;
    void setBidPrice(const qreal &value) noexcept;

    const qreal &getLow() const noexcept;
    void setLow(const qreal &value) noexcept;

    const qreal &getHigh() const noexcept;
    void setHigh(const qreal &value) noexcept;

    const qreal &getVolume() const noexcept;
    void setVolume(const qreal &value) noexcept;

    const qreal &getLastPrice() const noexcept;
    void setLastPrice(const qreal &value) noexcept;

    const qreal &getChange() const noexcept;
    void setChange(const qreal &value) noexcept;

    const qreal &getOpen() const noexcept;
    void setOpen(const qreal &value) noexcept;

    const qreal &getClose() const noexcept;
    void setClose(const qreal &value) noexcept;

    const qreal &getBaseVolume() const noexcept;
    void setBaseVolume(const qreal &value) noexcept;

    const qreal &getSpread() const noexcept;
    void setSpread(const qreal &value) noexcept;

    const qreal &getTradeFee() const noexcept;
    void setTradeFee(const qreal &value) noexcept;

    const qreal &getMinBasePrice() const noexcept;
    void setMinBasePrice(const qreal &value) noexcept;

    const qreal &getMaxBasePrice() const noexcept;
    void setMaxBasePrice(const qreal &value) noexcept;

    const qreal &getPriceTickSize() const noexcept;
    void setPriceTickSize(const qreal &value) noexcept;

    const qreal &getMinBaseVolume() const noexcept;
    void setMinBaseVolume(const qreal &value) noexcept;

    const qreal &getMaxBaseVolume() const noexcept;
    void setMaxBaseVolume(const qreal &value) noexcept;

    const qreal &getMinNotional() const noexcept;
    void setMinNotional(const qreal &value) noexcept;

    const qreal &getCurrentTrendRate() const noexcept;
    void setCurrentTrendRate(const qreal &value) noexcept;

    const qreal &getPercentageProfit() const noexcept;
    qreal getPercentageProfit100thPercent() const noexcept;
    void setPercentageProfit(const qreal &value) noexcept;

    const qreal &getMinTrend() const noexcept;
    void setMinTrend(const qreal &value) noexcept;

    const MarketId &getId() const noexcept;
    void setId(const MarketId &value) noexcept;

    bool getIsTradable() const noexcept;
    void setIsTradable(const bool value) noexcept;

    bool getIsVolumeTrading() const noexcept;
    void setIsVolumeTrading(const bool value) noexcept;

    bool getPauseMode() const noexcept;
    void setPauseMode(const bool value) noexcept;

    bool getSellMode() const noexcept;
    void setSellMode(const bool value) noexcept;

    bool getBuyMode() const noexcept;
    void setBuyMode(const bool value) noexcept;

    void setUserConfig(const Market::UserConfig &config) noexcept;
    const UserConfig &getUserConfig() const noexcept;

    void setConfig(const Market::Config &config) noexcept;

    const QJsonObject &getMetaConfig() const noexcept;
    void setMetaConfig(const QJsonObject &data) noexcept;

    QString getLabel() const noexcept;

    const qreal &getVolumeStepSize() const noexcept;
    void setVolumeStepSize(const qreal &value) noexcept;

    const qreal &getTradeSumm() const noexcept;
    void setTradeSumm(const qreal &value) noexcept;
signals:
    void statusChanged();
    void statusMessageChanged();
    void lastRateUpdateTimeChanged();
    void askPriceChanged();
    void bidPriceChanged();
    void lowChanged();
    void highChanged();
    void volumeChanged();
    void lastPriceChanged();
    void changeChanged();
    void openChanged();
    void closeChanged();
    void baseVolumeChanged();
    void spreadChanged();
    void tradeFeeChanged();
    void minBaseTradePriceChanged();
    void maxBaseTradePriceChanged();
    void priceTickChanged();
    void minBaseVolumeChanged();
    void maxBaseVolumeChanged();
    void minNotionalChanged();
    void currentTrendRateChanged();
    void percentageProfitChanged();
    void minTrendChanged();
    void idChanged();
    void isTradableChanged();
    void isVolumeTradingChanged();
    void pauseModeChanged();
    void sellModeChanged();
    void buyModeChanged();
    void metaConfigChanged();
    void tradeSummChanged();
private:
    Currency *getCurrencyPtr() const noexcept;
    Currency *getBaseCurrencyPtr() const noexcept;
private:
    Currency::ConstPtr                  m_tradeCurrency{};
    Currency::ConstPtr                  m_baseCurrency{};
    UserConfig                          m_userConfig{};
    Config                              m_config{};
};
#pragma pack (pop)
#endif // MARKET_H
