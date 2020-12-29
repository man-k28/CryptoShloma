#ifndef SELLORDERSPROXYMODEL_H
#define SELLORDERSPROXYMODEL_H
#include <QSortFilterProxyModel>

class SellOrdersProxyModel final: public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SellOrdersProxyModel(QObject *parent = nullptr) noexcept;
//    Q_INVOKABLE void refresh() noexcept;
protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const noexcept override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept override;
};

#endif // SELLORDERSPROXYMODEL_H
