#include "trade_history_proxy_model.h"
#include "trade_history_model.h"
#include <common/constants.h>

TradeHistoryProxyModel::TradeHistoryProxyModel(QObject *parent) noexcept :
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
}

void TradeHistoryProxyModel::setFilterOrderType(const Order::OrderType type) noexcept
{
    m_OrderTypeFilter = type;
    invalidateFilter();
}

void TradeHistoryProxyModel::setFilterMarketId(const Market::MarketId &id) noexcept
{
    m_marketId = id;
    invalidateFilter();
}

bool TradeHistoryProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const noexcept
{
    const auto & leftVariant = sourceModel()->data( left, TradeHistoryModel::OrderRole );
    const auto & rightVariant = sourceModel()->data( right, TradeHistoryModel::OrderRole );
    Q_ASSERT_X(leftVariant.canConvert< Order * >() && rightVariant.canConvert< Order * >(), "QVariant convert", "TradeHistoryProxyModel::lessThan" );

    const auto & leftOrder = leftVariant.value< Order * >();
    const auto & rightOrder = rightVariant.value< Order * >();

    return leftOrder->getUpdatedTimeStamp() < rightOrder->getUpdatedTimeStamp();
}

bool TradeHistoryProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept
{
    const QModelIndex &rowIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    if ( !rowIndex.isValid() ) {
        return false;
    }
    const auto &variant = sourceModel()->data( rowIndex, TradeHistoryModel::OrderRole );
    Q_ASSERT_X(variant.canConvert< Order * >(), "QVariant convert", "TradeHistoryProxyModel::filterAcceptsRow" );

    const auto &order = variant.value< Order * >();
    if ( m_marketId.isValid() )
        return order->getMarket()->getId() == m_marketId;
    else return true;
}

TradeHistorySelectedPairsProxyModel::TradeHistorySelectedPairsProxyModel(QObject *parent) noexcept
    : TradeHistoryProxyModel(parent)
{
}

bool TradeHistorySelectedPairsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept
{
    const QModelIndex &rowIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    if ( !rowIndex.isValid() ) {
        return false;
    }
    const auto &variant = sourceModel()->data( rowIndex, TradeHistoryModel::OrderRole );
    Q_ASSERT_X(variant.canConvert< Order * >(), "QVariant convert", "TradeHistorySelectedPairsProxyModel::filterAcceptsRow" );

    const auto &order = variant.value< Order * >();
    if ( m_marketId.isValid() )
        return order->getMarket()->getId() == m_marketId;
    else return false;
}
