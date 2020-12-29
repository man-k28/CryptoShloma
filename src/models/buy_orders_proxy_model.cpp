#include "buy_orders_proxy_model.h"
#include "market_orders_model.h"

BuyOrdersProxyModel::BuyOrdersProxyModel(QObject *parent) noexcept :
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
}

//void BuyOrdersProxyModel::refresh() noexcept
//{
//    qobject_cast<MarketOrdersModel*>(sourceModel())->refresh();
//}

bool BuyOrdersProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const noexcept
{
    const auto & leftVariant = sourceModel()->data( left, MarketOrdersModel::OrderRole );
    const auto & rightVariant = sourceModel()->data( right, MarketOrdersModel::OrderRole );
    Q_ASSERT_X(leftVariant.canConvert< Order * >() && rightVariant.canConvert< Order * >(), "QVariant convert", "BuyOrdersProxyModel::lessThan" );

    const auto & leftOrder = leftVariant.value< Order * >();
    const auto & rightOrder = rightVariant.value< Order * >();

    return leftOrder->getPrice() < rightOrder->getPrice(); //FIXME: double compare
}

bool BuyOrdersProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept
{
    const QModelIndex &buyTypeIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    if ( !buyTypeIndex.isValid() ) {
        return false;
    }
    const auto &variant = sourceModel()->data( buyTypeIndex, MarketOrdersModel::OrderRole );
    Q_ASSERT_X(variant.canConvert< Order * >(), "QVariant convert", "BuyOrdersProxyModel::filterAcceptsRow" );

    const auto &order = variant.value< Order * >();

    return ( order->getType() == Order::Buy);
}
