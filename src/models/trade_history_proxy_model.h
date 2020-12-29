#ifndef TRADEHISTORYPROXYMODEL_H
#define TRADEHISTORYPROXYMODEL_H
#include <QSortFilterProxyModel>
#include <base/order.h>

class TradeHistoryProxyModel;
#pragma pack (push, 1)
class TradeHistoryProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit TradeHistoryProxyModel(QObject *parent = nullptr) noexcept;
public slots:
    void setFilterOrderType(const Order::OrderType type)  noexcept;
    void setFilterMarketId(const Market::MarketId &id = Market::MarketId()) noexcept;
protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const noexcept override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept override;
protected:
    Market::MarketId    m_marketId{};
private:
    Order::OrderType    m_OrderTypeFilter{Order::None};
};

class TradeHistorySelectedPairsProxyModel final: public TradeHistoryProxyModel
{
    Q_OBJECT
public:
    explicit TradeHistorySelectedPairsProxyModel(QObject *parent = nullptr) noexcept;
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept override;
};
#pragma pack (pop)
#endif // TRADEHISTORYPROXYMODEL_H
