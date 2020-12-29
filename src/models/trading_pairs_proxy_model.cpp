#include "trading_pairs_proxy_model.h"
#include "markets_model.h"

TradingPairsProxyModel::TradingPairsProxyModel(QObject *parent) noexcept :
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

bool TradingPairsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept
{
    const QModelIndex &indexRow = sourceModel()->index(sourceRow, 0, sourceParent);
    if ( !indexRow.isValid() ) {
        return false;
    }
    const auto &variant = sourceModel()->data( indexRow, MarketsModel::MarketRole );
    Q_ASSERT_X(variant.canConvert< Market * >(), "QVariant convert", "TradingPairsProxyModel::filterAcceptsRow" );
    const auto &market = variant.value< Market * >();

    return market->getIsTradable();
}
