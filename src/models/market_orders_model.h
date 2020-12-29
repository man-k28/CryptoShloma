#ifndef MARKETORDERSMODEL_H
#define MARKETORDERSMODEL_H
#include <QAbstractListModel>
#include <api/i_exchange_api_entry.h>

class MarketOrdersModel;
class ExchangeEntry;
//TODO: сделать final
class MarketOrdersModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum MarketOrdersModelRoles {
        OrderRole = Qt::UserRole + 1
    };

    explicit MarketOrdersModel(ExchangeEntry *parent = nullptr) noexcept;
    explicit MarketOrdersModel(const MarketOrdersModel &o) noexcept = delete;
    MarketOrdersModel& operator=(const MarketOrdersModel &o) noexcept = delete;
private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const noexcept override;
    QVariant data(const QModelIndex &index, int role = Qt::UserRole) const noexcept override;
    QHash<int, QByteArray> roleNames() const noexcept override;

public slots:
    void insert(const Market::MarketId &id) noexcept;
    void merge(const QList<exchange::OrderEntry> &data) noexcept;
    void remove(const Market::MarketId &id) noexcept;
    void refresh() noexcept;
private:
    Order::Container m_Data{};
    QList<Market::MarketId> m_Filters{};
    ExchangeEntry const *m_entry;
};
#endif // MARKETORDERSMODEL_H
