#ifndef TRADINGPAIRSPROXYMODEL_H
#define TRADINGPAIRSPROXYMODEL_H
#include <QSortFilterProxyModel>

class TradingPairsProxyModel final : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit TradingPairsProxyModel(QObject *parent = nullptr) noexcept;
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept override;
};

#endif // TRADINGPAIRSPROXYMODEL_H
