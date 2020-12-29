#ifndef OPENORDERSPROXYMODEL_H
#define OPENORDERSPROXYMODEL_H
#include <QSortFilterProxyModel>
#include <base/order.h>

class OpenOrdersProxyModel;
#pragma pack (push, 1)
class OpenOrdersProxyModel final: public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit OpenOrdersProxyModel(QObject *parent = nullptr) noexcept;
public slots:
    void setFilterOrderType(const Order::OrderType type) noexcept;
    void setFilterMarketId(const Market::MarketId &id) noexcept;
//    void refresh() noexcept;
protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const noexcept override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept override;
private:
    Order::OrderType    m_OrderTypeFilter{Order::None};
    Market::MarketId    m_marketId{};
};
#pragma pack (pop)

#endif // OPENORDERSPROXYMODEL_H
