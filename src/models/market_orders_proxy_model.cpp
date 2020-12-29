#include "market_orders_proxy_model.h"
#include "market_orders_model.h"

MarketOrdersProxyModel::MarketOrdersProxyModel(QObject *parent) noexcept :
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void MarketOrdersProxyModel::setFilterMarketId(const Market::MarketId &id) noexcept
{
    m_marketId = id;
    invalidateFilter();
}

bool MarketOrdersProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept
{
    const QModelIndex &rowIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    if ( !rowIndex.isValid() ) {
        return false;
    }
    const auto &variant = sourceModel()->data( rowIndex, MarketOrdersModel::OrderRole );
    Q_ASSERT_X(variant.canConvert< Order * >(), "QVariant convert", "MarkerOrdersProxyModel::filterAcceptsRow" );

    const auto &order = variant.value< Order * >();

    if ( m_marketId.isValid() )
        return order->getMarket()->getId() == m_marketId;
    else return false;
}
