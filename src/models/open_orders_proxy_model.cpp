#include "open_orders_proxy_model.h"
#include "open_orders_model.h"

OpenOrdersProxyModel::OpenOrdersProxyModel(QObject *parent) noexcept :
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
}

void OpenOrdersProxyModel::setFilterOrderType(const Order::OrderType type) noexcept {
    m_OrderTypeFilter = type;
    invalidateFilter();
}

void OpenOrdersProxyModel::setFilterMarketId(const Market::MarketId &id) noexcept
{
    m_marketId = id;
    invalidateFilter();
}

//void OpenOrdersProxyModel::refresh() noexcept
//{
//    qobject_cast<OpenOrdersModel*>(sourceModel())->refresh();
//}

bool OpenOrdersProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const noexcept
{
    const auto & leftVariant = sourceModel()->data( left, OpenOrdersModel::OrderRole );
    const auto & rightVariant = sourceModel()->data( right, OpenOrdersModel::OrderRole );
    Q_ASSERT_X(leftVariant.canConvert< Order * >() && rightVariant.canConvert< Order * >(), "QVariant convert", "OpenOrdersProxyModel::lessThan" );

    const auto & leftOrder = leftVariant.value< Order * >();
    const auto & rightOrder = rightVariant.value< Order * >();

    return leftOrder->getTimeStamp() < rightOrder->getTimeStamp();
}

bool OpenOrdersProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept
{
    const QModelIndex &rowIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    if ( !rowIndex.isValid() ) {
        return false;
    }
    const auto &variant = sourceModel()->data( rowIndex, OpenOrdersModel::OrderRole );
    Q_ASSERT_X(variant.canConvert< Order * >(), "QVariant convert", "OpenOrdersProxyModel::filterAcceptsRow" );

    const auto &order = variant.value< Order * >();

    //TODO: пределать фильтр

//    bool isPatternOk = false;
    bool isFilterSymbol = true;

//    const QString &pattern = filterRegExp().pattern();
//    if ( !pattern.isEmpty() ) {
//        const quint32 intPattern = pattern.toUInt(&isPatternOk);
//        if ( isPatternOk && intPattern ) {
//            const quint32 comp = order->getMarket()->getId().get();
//            if (intPattern != comp)
//                isFilterSymbol = false;
//        } else {
//            const QString &comp = order->getMarket()->getCurrency()->getSymbol();
//            if (pattern.compare(comp))
//                isFilterSymbol = false;
//        }
//    }

    if ( !isFilterSymbol || ( m_OrderTypeFilter == Order::None ) )
        return isFilterSymbol;

    if ( order->getType() == m_OrderTypeFilter )
        isFilterSymbol = true;
    else
        isFilterSymbol = false;

    return isFilterSymbol;
}
