#ifndef OPENORDERSMODEL_H
#define OPENORDERSMODEL_H
#include <QAbstractListModel>
#include <api/i_exchange_api_entry.h>

class OpenOrdersModel;
class ExchangeEntry;
//TODO: сделать final
class OpenOrdersModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum OpenOrdersModelRoles {
        OrderRole = Qt::UserRole + 1
    };

    explicit OpenOrdersModel(ExchangeEntry *parent = nullptr) noexcept;
    explicit OpenOrdersModel(const OpenOrdersModel &o) noexcept = delete;
    OpenOrdersModel& operator=(const OpenOrdersModel &o) noexcept = delete;
private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const noexcept override;
    QVariant data(const QModelIndex &index, int role = Qt::UserRole) const noexcept override;
    QHash<int, QByteArray> roleNames() const noexcept override;

public slots:
    void merge(const QList<exchange::OrderEntry> &data) noexcept;
    void remove(const QList<Order::OrderId> &data) noexcept;
    void refresh() noexcept;
private:
    Order::Container m_Data{};
    ExchangeEntry const * m_entry;
};
#endif // OPENORDERSMODEL_H
