#ifndef BALANCESPROXYMODEL_H
#define BALANCESPROXYMODEL_H
#include <QSortFilterProxyModel>

class BalancesProxyModel final: public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit BalancesProxyModel(QObject *parent = nullptr) noexcept;
//    Q_INVOKABLE void refresh() noexcept;
//    void setFilterMarketId(const Market::MarketId &id) noexcept;
protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const noexcept override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept override;
//private:
//    Market::MarketId    m_marketId;
};
#endif // BALANCESPROXYMODEL_H
