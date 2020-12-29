#ifndef SPREADMARKETSPROXYMODEL_H
#define SPREADMARKETSPROXYMODEL_H
#include <QSortFilterProxyModel>

class SpreadMarketsProxyModel final : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SpreadMarketsProxyModel(QObject *parent = nullptr) noexcept;
protected:
    bool lessThan( const QModelIndex & left, const QModelIndex & right ) const noexcept override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept override;
};

#endif // SPREADMARKETSPROXYMODEL_H
