#ifndef BUYORDERSPROXYMODEL_H
#define BUYORDERSPROXYMODEL_H
#include <QSortFilterProxyModel>

class BuyOrdersProxyModel final : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit BuyOrdersProxyModel(QObject *parent = nullptr) noexcept;
//    Q_INVOKABLE void refresh() noexcept;
protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const noexcept override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept override;
};

#endif // BUYORDERSPROXYMODEL_H
