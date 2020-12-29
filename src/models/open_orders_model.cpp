#include "open_orders_model.h"
#include "exchange_pool.h"
#include <Logger.h>
#include "markets_model.h"

OpenOrdersModel::OpenOrdersModel(ExchangeEntry *parent) noexcept
    : QAbstractListModel(parent)
    , m_entry(parent)
{}

int OpenOrdersModel::rowCount(const QModelIndex &parent) const noexcept
{
    return parent.isValid() ? 0 :m_Data.count();
}

QVariant OpenOrdersModel::data(const QModelIndex &index, int role) const noexcept
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

QHash<int, QByteArray> OpenOrdersModel::roleNames() const noexcept
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[OrderRole] = "order";
    return roles;
}

void OpenOrdersModel::merge(const QList<exchange::OrderEntry> &data) noexcept
{
    LOG_TRACE_TIME();

    Order::Container filtered;
    for (const auto &item : data) {
        std::copy_if(m_Data.begin(), m_Data.end(), std::back_inserter(filtered), [&item, &filtered](const Order::Ptr &order){
            return item.marketId == order->getMarket()->getId() && !filtered.contains(order);
        });
    }

    for (auto item = filtered.begin(); item != filtered.end();) {
        auto found = std::find_if(data.cbegin(), data.cend(),
            [ &item ]( const exchange::OrderEntry &order ) noexcept
            {
                return item->get()->getOrderId() == order.params.orderId ;
            });
        if ( found == data.cend() )
            item = filtered.erase(item);
        else
            ++item;
    }

    if ( filtered.count() ) {
        for (auto item = filtered.begin(); item != filtered.end();++item) {
            const qint32 index = m_Data.indexOf(*item);
            beginRemoveRows(QModelIndex(), index, index);
            m_Data.removeOne(*item);
            endRemoveRows();
        }
    }

    for (const auto &item : data) {
        auto found = std::find_if(m_Data.begin(), m_Data.end(),
            [ &item ]( const decltype( m_Data )::value_type &order ) noexcept
            {
                return order->getOrderId() == item.params.orderId;
            });
        if ( found == m_Data.end() ) {
            auto market = m_entry->getMarketsModel()->findMarketById(item.marketId);
            if ( !market.isNull() ) {
                const int index = m_Data.count();
                beginInsertRows(QModelIndex(), index, index);
                auto order = Order::Ptr::create(item.params);
                order->setMarket(market);
                m_Data.append(order);
                endInsertRows();
            }
        } else {
            if ( item.params.updatedTimeStamp >= found->get()->getUpdatedTimeStamp() ) {
                found->get()->setConfig(item.params);
                const qint32 indexI = static_cast<qint32>(std::distance(m_Data.begin(), found));
                const QModelIndex &indexRow = this->index(indexI);
                emit dataChanged(indexRow, indexRow, QVector<int>() << OrderRole);
            }
        }
    }
}

void OpenOrdersModel::remove(const QList<Order::OrderId> &data) noexcept
{
    LOG_TRACE_TIME();

    for (auto item = m_Data.begin(); item != m_Data.end();) {
        auto found = std::find_if(data.cbegin(), data.cend(),
            [ &item ]( const Order::OrderId &id ) noexcept
            {
                return item->get()->getOrderId() == id;
            });
        if ( found != data.cend() ) {
            const qint32 index = static_cast<qint32>(std::distance(m_Data.begin(), item));
            beginRemoveRows(QModelIndex(),index,index);
            item = m_Data.erase(item);
            endRemoveRows();
        } else
            ++item;
    }
}

void OpenOrdersModel::refresh() noexcept
{
    beginResetModel();
    endResetModel();
}
