#include "markets_model.h"
#include <Logger.h>

MarketsModel::MarketsModel(QObject *parent) noexcept
    : QAbstractListModel(parent)
{
    connect(this, &MarketsModel::removeTradePairIdFromTrading, this, &MarketsModel::cancelTradingMarket);
}

const Market::ConstPtr MarketsModel::findMarketById(const Market::MarketId &id) const noexcept
{
    auto found = std::find_if(m_Data.begin(), m_Data.end(), [&id](const Market::Ptr &item)
    {
        return item->getId() == id;
    });
    if ( found == m_Data.end() )
        return {};
    return *found;
}

int MarketsModel::rowCount(const QModelIndex &parent) const noexcept
{
    return parent.isValid() ? 0 : m_Data.count();
}

QVariant MarketsModel::data(const QModelIndex &index, int role) const noexcept
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
        case MarketRole: return QVariant::fromValue(item.get());
        case IsTradableRole: return item->getIsTradable();
        default: return {};
    }
}

bool MarketsModel::setData(const QModelIndex &index, const QVariant &value, int role) noexcept
{
    if (!index.isValid()
            || ( index.row() < 0 )
            || ( index.row() >= m_Data.count() )
    )
        return false;

    const auto &item = m_Data.at(index.row());
    if (item.isNull())
        return false;

    switch (role) {
        case IsTradableRole:
            item->setIsTradable(value.toBool());
            emit dataChanged(index, index, QVector<int>() << IsTradableRole);
            break;
        default: return false;
    }
    return true;
}

QHash<int, QByteArray> MarketsModel::roleNames() const noexcept
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[MarketRole] = "market";
    roles[IsTradableRole] = "isTradable";
    return roles;
}

void MarketsModel::mergeMarketData(const QList<exchange::MarketEntry> &data) noexcept
{
    LOG_TRACE_TIME();
    for (const auto &item : data) {
        auto found = std::find_if(m_Data.begin(), m_Data.end(),
            [ & ]( const decltype( m_Data )::value_type &market ) noexcept
            {
                return market->getId() == item.params.id;
            });
        if ( found != m_Data.end() ) {
            found->get()->setConfig(item.params);
            const qint32 indexI = static_cast<qint32>(std::distance(m_Data.begin(), found));
            const QModelIndex &indexRow = this->index(indexI);
            emit dataChanged(indexRow, indexRow, QVector<int>() << MarketRole);
        }
    }
    emit mergeCompeleted();
}

void MarketsModel::refresh() noexcept
{
    beginResetModel();
    endResetModel();
}

void MarketsModel::updateMarketConfig(const Market::MarketId &marketId, const Market::UserConfig &config) noexcept
{
    auto found = std::find_if(m_Data.begin(), m_Data.end(), [&marketId](const Market::ConstPtr &item){
        return marketId == item->getId();
    });

    if ( found != m_Data.end() ) {
        if ( !(found->get()->getUserConfig() == config)) {
            const qint32 indexI = static_cast<qint32>(std::distance(m_Data.begin(), found));
            const QModelIndex &indexRow = this->index(indexI);
            found->get()->setUserConfig(config);
            emit dataChanged(indexRow, indexRow, QVector<int>() << MarketRole << IsTradableRole);
        }
    }
}

void MarketsModel::cancelTradingMarket(const Market::MarketId &id) noexcept
{
    for (auto market : m_Data) {
        if ( market->getId() == id ) {
            market->setIsTradable(false);
            break;
        }
    }
}

const Currency::ConstPtr MarketsModel::findCurrencyById(const Currency::CurrencyId &id) const noexcept
{

    auto found = std::find_if(m_Currencys.begin(), m_Currencys.end(), [&id](const Currency::Ptr &item)
    {
        return item->getId() == id;
    });
    if ( found == m_Currencys.end() )
        return {};
    return *found;
}

void MarketsModel::mergeMarketInfo(const QList<exchange::MarketEntry> &data) noexcept
{
    LOG_TRACE_TIME();
    for (auto item = m_Data.begin(); item != m_Data.end();) {
        auto found = std::find_if(data.cbegin(), data.cend(),
            [ &item ]( const exchange::MarketEntry &market ) noexcept
            {
                return market.params.id == item->get()->getId();
            });
        if ( found == data.cend() ) {
            const qint32 index = static_cast<qint32>(std::distance(m_Data.begin(), item));
            beginRemoveRows(QModelIndex(),index,index);
            item = m_Data.erase(item);
            endRemoveRows();
        }
        else
            ++item;
    }

    for (const auto &item : data) {
        auto found = std::find_if(m_Data.begin(), m_Data.end(),
            [ &item ]( const decltype( m_Data )::value_type &market ) noexcept
            {
                return market->getId() == item.params.id;
            });
        if ( found != m_Data.end() ) {
            found->get()->setConfig(item.params);
            const qint32 indexI = static_cast<qint32>(std::distance(m_Data.begin(), found));
            const QModelIndex &indexRow = this->index(indexI);
            emit dataChanged(indexRow, indexRow, QVector<int>() << MarketRole);
        }
        else {
            const auto &currency = findCurrencyById(item.tradeCurrencyId);
            const auto &baseCurrency = findCurrencyById(item.baseCurrencyId);

            if ( currency.isNull() || baseCurrency.isNull() )
                continue;
            const int count = m_Data.count();
            beginInsertRows(QModelIndex(), count, count);

            auto market = Market::Ptr::create(item.params);
            market->setCurrency(currency);
            market->setBaseCurrency(baseCurrency);
            m_Data.append(market);
            endInsertRows();
        }
    }
}

void MarketsModel::mergeCurrencies(const QList<Currency::Config> &data) noexcept
{
    LOG_TRACE_TIME();

    for (auto item = m_Currencys.begin(); item != m_Currencys.end();) {
        auto found = std::find_if(data.cbegin(), data.cend(),
            [ &item ]( const Currency::Config &currency ) noexcept
            {
                return currency.id == item->get()->getId();
            });
        if ( found == data.cend() )
            item = m_Currencys.erase(item);
        else
            ++item;
    }

    for (const auto &item : data) {
        auto found = std::find_if(m_Currencys.begin(), m_Currencys.end(),
            [ &item ]( const decltype( m_Currencys )::value_type &currency ) noexcept
            {
                return currency->getId() == item.id;
            });
        if ( found != m_Currencys.end() ) {
            found->get()->setConfig(item);
        }
        else
            m_Currencys.append(Currency::Ptr::create(item));
    }
}
