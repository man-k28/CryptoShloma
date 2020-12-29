#ifndef BALANCESMODEL_H
#define BALANCESMODEL_H
#include <QAbstractListModel>
#include <api/i_exchange_api_entry.h>

class BalancesModel;
class ExchangeEntry;
//TODO: сделать final, убрать include balance
class BalancesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum BalancesModelRoles {
        BalanceRole = Qt::UserRole + 1
    };

    explicit BalancesModel(ExchangeEntry *parent = nullptr) noexcept;
    explicit BalancesModel(const BalancesModel &o) noexcept = delete;
    BalancesModel& operator=(const BalancesModel &o) noexcept = delete;
private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const noexcept override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const noexcept override;
    QHash<int, QByteArray> roleNames() const noexcept override;
public slots:
    void merge(const QList<exchange::BalanceEntry> &data) noexcept;
    void refresh() noexcept;
private:
    QList<Balance::Ptr> m_Data{};
    ExchangeEntry const * m_entry;
};
#endif // BALANCESMODEL_H
