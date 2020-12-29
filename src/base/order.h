#ifndef ORDER_H
#define ORDER_H

#include "market.h"

class Order;
#pragma pack (push, 1)
class Order : public QObject, public Boilerplate::BaseEntry<Order, quint64>
{
    Q_OBJECT
    Q_PROPERTY(QDateTime timestamp READ getTimeStamp NOTIFY timeStampChanged)
    Q_PROPERTY(QDateTime updatedTimestamp READ getUpdatedTimeStamp NOTIFY updatedTimeStampChanged)
    Q_PROPERTY(Market *market READ getMarketPtr CONSTANT)
    Q_PROPERTY(qreal price READ getPrice NOTIFY priceChanged)
    Q_PROPERTY(qreal currencyVolume READ getCurrencyVolume NOTIFY currencyVolumeChanged)
    Q_PROPERTY(qreal baseVolume READ getBaseVolume NOTIFY baseVolumeChanged)
    Q_PROPERTY(qreal fee READ getFee NOTIFY feeChanged)
    Q_PROPERTY(qreal remaining READ getRemaining NOTIFY remainingChanged)
    Q_PROPERTY(Order::OrderId orderId READ getOrderId NOTIFY orderIdChanged)
    Q_PROPERTY(Order::OrderType type READ getType NOTIFY typeChanged) //TODO зарегистрировать в qml
public:
    enum OrderType {
        Buy = 1,
        Sell,
        None
    };
    Q_ENUM(OrderType)

    using OrderId = Id;
public:
    struct Config
    {
        QDateTime           timeStamp{};
        QDateTime           updatedTimeStamp{};
        qreal               price{0.};
        qreal               currencyVolume{0.};
        qreal               baseVolume{0.};
        qreal               fee{0.};
        qreal               remaining{0.};
        OrderId             orderId{};
        OrderType           type{Buy};
    };
public:
    explicit Order(QObject *parent = nullptr);
    explicit Order(const Config &config);
    Order(const Order &o) noexcept;
    Order(Order &&o) noexcept;

    static QString toOrderTypeString(OrderType type) noexcept;

    const Market::ConstPtr &getMarket() const noexcept;
    void setMarket(const Market::ConstPtr &value) noexcept;

    const QDateTime &getTimeStamp() const noexcept;
    void setTimeStamp(const QDateTime &value) noexcept;

    const QDateTime &getUpdatedTimeStamp() const noexcept;
    void setUpdatedTimeStamp(const QDateTime &value) noexcept;

    const qreal &getPrice() const noexcept;
    void setPrice(const qreal &value) noexcept;

    const qreal &getCurrencyVolume() const noexcept;
    void setCurrencyVolume(const qreal &value) noexcept;

    const qreal &getBaseVolume() const noexcept;
    void setBaseVolume(const qreal &value) noexcept;

    const qreal &getFee() const noexcept;
    void setFee(const qreal &value) noexcept;

    const qreal &getRemaining() const noexcept;
    void setRemaining(const qreal &value) noexcept;

    const OrderId &getOrderId() const noexcept;
    void setOrderId(const OrderId &value) noexcept;

    OrderType getType() const noexcept;
    void setType(const OrderType value) noexcept;

    void setConfig(const Config &config) noexcept;
signals:
    void timeStampChanged();
    void updatedTimeStampChanged();
    void priceChanged();
    void currencyVolumeChanged();
    void baseVolumeChanged();
    void feeChanged();
    void remainingChanged();
    void orderIdChanged();
    void typeChanged();
private:
    Market *getMarketPtr() const noexcept;
private:
    Config              m_config{};
    Market::ConstPtr    m_market{};
};
#pragma pack (pop)
#endif // ORDER_H
