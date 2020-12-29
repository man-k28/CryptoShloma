#ifndef VOLUMEMARKETSPROXYMODEL_H
#define VOLUMEMARKETSPROXYMODEL_H
#include <QSortFilterProxyModel>

class VolumeMarketsProxyModel final : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit VolumeMarketsProxyModel(QObject *parent = nullptr) noexcept;
protected:
    bool lessThan( const QModelIndex & left, const QModelIndex & right ) const noexcept override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept override;
};

#endif // VOLUMEMARKETSPROXYMODEL_H
