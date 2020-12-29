#include "balances_proxy_model.h"
#include "balances_model.h"

BalancesProxyModel::BalancesProxyModel(QObject *parent) noexcept :
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::AscendingOrder);
}

//void BalancesProxyModel::refresh() noexcept
//{
//    qobject_cast<BalancesModel*>(sourceModel())->refresh();
//}

//void BalancesProxyModel::setFilterMarketId(const Market::MarketId &id) noexcept
//{
//    m_marketId = id;
//    invalidateFilter();
//}

bool BalancesProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const noexcept
{
    const auto & leftVariant = sourceModel()->data( left, BalancesModel::BalanceRole );
    const auto & rightVariant = sourceModel()->data( right, BalancesModel::BalanceRole );
    Q_ASSERT_X(leftVariant.canConvert< Balance * >() && rightVariant.canConvert< Balance * >(), "QVariant convert", "BalancesProxyModel::lessThan" );

    const auto & leftBalance = leftVariant.value< Balance * >();
    const auto & rightBalance = rightVariant.value< Balance * >();

    return leftBalance->getCurrency()->getId() < rightBalance->getCurrency()->getId();
}

bool BalancesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept
{
    const QModelIndex &rowIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    if ( !rowIndex.isValid() ) {
        return false;
    }
    const auto &variant = sourceModel()->data( rowIndex, BalancesModel::BalanceRole );
    Q_ASSERT_X(variant.canConvert< Balance * >(), "QVariant convert", "BalancesProxyModel::filterAcceptsRow" );

    const auto &balance = variant.value< Balance * >();

    return !qFuzzyIsNull(balance->getTotal());
}
