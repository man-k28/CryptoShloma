#include "balances_model.h"
#include "markets_model.h"
#include "exchange_pool.h"
#include <Logger.h>

BalancesModel::BalancesModel(ExchangeEntry *parent) noexcept
    : QAbstractListModel(parent)
    , m_entry(parent)
{
}

int BalancesModel::rowCount(const QModelIndex &parent) const noexcept
{
    return parent.isValid() ? 0 : m_Data.count();
}

QVariant BalancesModel::data(const QModelIndex &index, int role) const noexcept
{
    if (!index.isValid()
            || ( index.row() < 0 )
            || ( index.row() >= m_Data.count() )
    )
        return {};
    const auto &item = m_Data.at(index.row());

    if (item.isNull())
        return {};
    switch (role) {
        case BalanceRole: return QVariant::fromValue(item.get());
        default: return {};
    }
}

QHash<int, QByteArray> BalancesModel::roleNames() const noexcept
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[BalanceRole] = "balance";
    return roles;
}

void BalancesModel::merge(const QList<exchange::BalanceEntry> &data) noexcept
{
    LOG_TRACE_TIME();

//    for (auto it = m_Data.begin(); it != m_Data.end();) {
//        auto found = std::find_if(data.begin(), data.end(),
//            [ &it ]( const exchange::BalanceEntry &entry ) noexcept {
//                return entry.currencyId == (*it)->getCurrency()->getId();
//            });

//        if ( found == data.cend() ) {
//            const qint32 index = static_cast<qint32>(std::distance(m_Data.begin(), it));
//            beginRemoveRows(QModelIndex(),index,index);
//            it = m_Data.erase(it);
//            endRemoveRows();
//        }
//        else
//            ++it;
//    }

    for (const auto &item : data) {
        auto found = std::find_if(m_Data.begin(), m_Data.end(),
            [ &item ]( const decltype( m_Data )::value_type &entry ) noexcept {
                return entry->getCurrency()->getId() == item.currencyId;
            });
        if ( found != m_Data.end() ) {
            if ( item.params.timestamp >= found->get()->getTimestamp() ) {
                found->get()->setConfig(item.params);
                const qint32 indexI = static_cast<qint32>(std::distance(m_Data.begin(), found));
                const QModelIndex &indexRow = this->index(indexI);
                emit dataChanged(indexRow, indexRow, QVector<int>() << BalanceRole);
            }
        }
        else {
            const auto &currency = m_entry->getMarketsModel()->findCurrencyById(item.currencyId);

            if ( currency.isNull())
                continue;
            const int count = m_Data.count();
            beginInsertRows(QModelIndex(), count, count);
            auto balance = Balance::Ptr::create(item.params);
            balance->setCurrency(currency);
            m_Data.append(balance);
            endInsertRows();
        }
    }
}

void BalancesModel::refresh() noexcept
{
    beginResetModel();
    endResetModel();
}
