#ifndef MARKERORDERSPROXYMODEL_H
#define MARKERORDERSPROXYMODEL_H
#include <QSortFilterProxyModel>
#include <base/market.h>

class MarketOrdersProxyModel;
#pragma pack (push, 1)
class MarketOrdersProxyModel final: public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit MarketOrdersProxyModel(QObject *parent = nullptr) noexcept;
public slots:
    void setFilterMarketId(const Market::MarketId &id = Market::MarketId()) noexcept;
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const noexcept override;
private:
    Market::MarketId    m_marketId{};
};
#pragma pack (pop)
#endif // MARKERORDERSPROXYMODEL_H
