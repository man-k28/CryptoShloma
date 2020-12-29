#include "spread_markets_proxy_model.h"
#include "markets_model.h"
#include <common/settings_manager.h>

SpreadMarketsProxyModel::SpreadMarketsProxyModel(QObject *parent) noexcept :
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
}

bool SpreadMarketsProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const noexcept
{
    const auto & leftVariant = sourceModel()->data( left, MarketsModel::MarketRole );
    const auto & rightVariant = sourceModel()->data( right, MarketsModel::MarketRole );
    Q_ASSERT_X(leftVariant.canConvert< Market * >() && rightVariant.canConvert< Market * >(), "QVariant convert", "SpreadMarketsProxyModel::lessThan" );

    const auto & leftMarket = leftVariant.value< Market * >();
    const auto & rightMarket = rightVariant.value< Market * >();

    return leftMarket->getSpread() < rightMarket->getSpread(); //FIXME: double compare
}

bool SpreadMarketsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept
{
    const QModelIndex &indexRow = sourceModel()->index(sourceRow, 0, sourceParent);
    if ( !indexRow.isValid() ) {
        return false;
    }
    const auto &variant = sourceModel()->data( indexRow, MarketsModel::MarketRole );
    Q_ASSERT_X(variant.canConvert< Market * >(), "QVariant convert", "SpreadMarketsProxyModel::filterAcceptsRow" );

    const auto &market = variant.value< Market * >();
    bool filterText = true;
    const auto &pattern = filterRegExp().pattern();
    if ( !pattern.isEmpty() )
        filterText = market->getId().get().contains(pattern, Qt::CaseInsensitive);

    return ( market->getBaseVolume() > SettingsManager::get(SettingsManager::MinBaseVolume).toReal() ) &&
           ( market->getSpread() > 0 ) &&
           ( market->getStatus() == Market::Ok ) &&
           ( !market->getIsTradable() &&
             filterText);
}
