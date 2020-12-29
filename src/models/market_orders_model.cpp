#include "market_orders_model.h"
#include "markets_model.h"
#include "exchange_pool.h"
#include <Logger.h>

MarketOrdersModel::MarketOrdersModel(ExchangeEntry *parent) noexcept
    : QAbstractListModel(parent)
    , m_entry(parent)
{}

int MarketOrdersModel::rowCount(const QModelIndex &parent) const noexcept
{
    return parent.isValid() ? 0 : m_Data.count();
}

QVariant MarketOrdersModel::data(const QModelIndex &index, int role) const noexcept
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
    case OrderRole: return QVariant::fromValue(item.get());
        default: return {};
    }
}

QHash<int, QByteArray> MarketOrdersModel::roleNames() const noexcept
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[OrderRole] = "order";
    return roles;
}

void MarketOrdersModel::insert(const Market::MarketId &id) noexcept
{
    if ( !m_Filters.contains(id) )
        m_Filters << id;
}

void MarketOrdersModel::merge(const QList<exchange::OrderEntry> &data) noexcept
{
    LOG_TRACE_TIME();
    QList<Market::MarketId> ids;
    for (auto i = data.cbegin(); i != data.cend(); ++i) {
        const auto &id = (*i).marketId;
        if ( !ids.contains(id) )
            ids << id;
    }

    for (auto item = m_Data.begin(); item != m_Data.end();) {
        if ( ids.contains(item->get()->getMarket()->getId())) {
            const qint32 index = static_cast<qint32>(std::distance(m_Data.begin(), item));
            beginRemoveRows(QModelIndex(),index,index);
            item = m_Data.erase(item);
            endRemoveRows();
        } else
            ++item;
    }

    for (auto item : data ) {
        if ( m_Filters.contains(item.marketId) ) {
            auto market = m_entry->getMarketsModel()->findMarketById(item.marketId);
            if ( !market.isNull() ) {
                const int index = m_Data.count();
                beginInsertRows(QModelIndex(), index, index);
                auto order = Order::Ptr::create(item.params);
                order->setMarket(market);
                m_Data.append(order);
                endInsertRows();
            }
        }
    }
}

void MarketOrdersModel::remove(const Market::MarketId &id) noexcept
{
    LOG_TRACE_TIME();
    m_Filters.removeAll(id);
    for (auto item = m_Data.begin(); item != m_Data.end();) {
        if ( item->get()->getMarket()->getId() == id ) {
            const qint32 index = static_cast<qint32>(std::distance(m_Data.begin(), item));
            beginRemoveRows(QModelIndex(),index,index);
            item = m_Data.erase(item);
            endRemoveRows();
        } else
            ++item;
    }
}

void MarketOrdersModel::refresh() noexcept
{
    beginResetModel();
    endResetModel();
}
