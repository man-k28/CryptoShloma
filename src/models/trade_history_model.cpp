#include "trade_history_model.h"
#include "exchange_pool.h"
#include <Logger.h>
#include "markets_model.h"

TradeHistoryModel::TradeHistoryModel(ExchangeEntry *parent) noexcept
    : QAbstractListModel(parent)
    , m_entry(parent)
{}

const Order::ConstPtr TradeHistoryModel::findByIndex(const qint32 index) const noexcept
{
    if ( index < m_Data.count())
        return m_Data.at(index);
    else return {};
}

int TradeHistoryModel::rowCount(const QModelIndex &parent) const noexcept
{
    return parent.isValid() ? 0 : m_Data.count();
}

QVariant TradeHistoryModel::data(const QModelIndex &index, int role) const noexcept
{
    if (!index.isValid()
            || ( index.row() < 0 )
            || ( index.row() >= m_Data.count() )
    )
        return QVariant();

    const auto &item = m_Data.at(index.row());

    if (item.isNull())
        return {};

    switch (role) {
        case OrderRole: return QVariant::fromValue(item.get());
        default: return {};
    }
}


QHash<int, QByteArray> TradeHistoryModel::roleNames() const noexcept
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[OrderRole] = "order";
    return roles;
}

void TradeHistoryModel::insert(const Market::MarketId &id) noexcept
{
    if ( !m_Filters.contains(id) )
        m_Filters << id;
}

void TradeHistoryModel::merge(const QList<exchange::OrderEntry> &data) noexcept
{
    LOG_TRACE_TIME();
    for (const auto &item : data) {
        auto found = std::find_if(m_Data.begin(), m_Data.end(),
            [ &item ]( const decltype( m_Data )::value_type &order ) noexcept
            {
                return order->getOrderId() == item.params.orderId;
            });
        if ( found == m_Data.end() ) {
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
}

void TradeHistoryModel::remove(const Market::MarketId &id) noexcept
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

void TradeHistoryModel::refresh() noexcept
{
    beginResetModel();
    endResetModel();
}
