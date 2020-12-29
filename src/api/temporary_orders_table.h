#ifndef TEMPORARYORDERSTABLE_H
#define TEMPORARYORDERSTABLE_H

#include <base/order.h>

class TemporaryOrdersTable final : public QObject
{
    Q_OBJECT
public:
    struct OrderRecord {
        /*!
         * \brief For create new
         */
        explicit OrderRecord(const Market::MarketId &marketId,
                             Order::OrderType type,
                             const qreal &price,
                             const qreal &baseVolume,
                             const qreal &volume = 0.) noexcept;
        /*!
         * \brief For update exist
         */
        explicit OrderRecord(const Market::MarketId &marketId,
                             const Order::OrderId &orderId,
                             const QDateTime &timestamp,
                             const qreal &price,
                             const qreal &baseVolume,
                             Order::OrderType type,
                             const qreal &volume) noexcept;
        Market::MarketId marketId = {};
        Order::OrderId orderId = {};
        Order::OrderType orderType = Order::Buy;
        qreal volume = 0.;
        qreal baseVolume = 0.;
        qreal price = 0.;
        QDateTime timestamp = {};
    };
public:
    explicit TemporaryOrdersTable(QObject *parent = nullptr) noexcept;
    bool submitOrder(const OrderRecord &record);
public slots:
    void updatePlaceOrder(const OrderRecord &record);
    void updateEraseOrder(const OrderRecord &record); //Для данных из submitOrder
    void updateEraseOrder(const Order::OrderId &orderId); //Для данных с истории торгов
private:
    QList<OrderRecord> m_data = {};
};

#endif // TEMPORARYORDERSTABLE_H
