#ifndef TRADEHISTORYMODEL_H
#define TRADEHISTORYMODEL_H
#include <QAbstractListModel>
#include <api/i_exchange_api_entry.h>

class TradeHistoryModel;
class ExchangeEntry;
//TODO: сделать final
class TradeHistoryModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum TradeHistoryModelRoles {
        OrderRole = Qt::UserRole + 1
    };

    explicit TradeHistoryModel(ExchangeEntry *parent = nullptr) noexcept;
    explicit TradeHistoryModel(const TradeHistoryModel &o) noexcept = delete;
    TradeHistoryModel& operator=(const TradeHistoryModel &o) noexcept = delete;
    const Order::ConstPtr findByIndex(const qint32 index) const noexcept;
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
    ExchangeEntry const * m_entry;
};
#endif // TRADEHISTORYMODEL_H
