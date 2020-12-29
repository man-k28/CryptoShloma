#ifndef EXCHANGE_POOL_H
#define EXCHANGE_POOL_H
#include <QAbstractListModel>
#include <algo/i_trade_algo.h>
#include <algo/i_analytics_algo.h>
#include <api/i_exchange_api.h>

class ITradeAlgorithm;
class QSortFilterProxyModel;
class MarketsModel;
class MarketOrdersModel;
class TradeHistoryModel;

class ExchangeEntry final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSortFilterProxyModel * marketOrdersProxyModel READ marketOrdersProxyModel CONSTANT)
    Q_PROPERTY(QSortFilterProxyModel * buyOrdersProxyModel READ buyOrdersProxyModel CONSTANT)
    Q_PROPERTY(QSortFilterProxyModel * sellOrdersProxyModel READ sellOrdersProxyModel CONSTANT)
    Q_PROPERTY(QSortFilterProxyModel * spreadMarketsProxyModel READ spreadMarketsProxyModel CONSTANT)
    Q_PROPERTY(QSortFilterProxyModel * volumeMarketsProxyModel READ volumeMarketsProxyModel CONSTANT)
    Q_PROPERTY(QSortFilterProxyModel * tradingPairsProxyModel READ tradingPairsProxyModel CONSTANT)
    Q_PROPERTY(QSortFilterProxyModel * openOrdersProxyModel READ openOrdersProxyModel CONSTANT)
    Q_PROPERTY(QSortFilterProxyModel * balancesProxyModel READ balancesProxyModel CONSTANT)
    Q_PROPERTY(QSortFilterProxyModel * tradeHistoryProxyModel READ tradeHistoryProxyModel CONSTANT)
    Q_PROPERTY(QSortFilterProxyModel * tradeHistorySelectedPairProxyModel READ tradeHistorySelectedPairProxyModel CONSTANT)
    Q_PROPERTY(IExchangeAPI * api READ getApi CONSTANT)
public:
    using Ptr = QSharedPointer< ExchangeEntry >;
    using Storage = QList< Ptr >;

    explicit ExchangeEntry (ITradeAlgorithm::Ptr algo,
                            const QList<AbstractAnalyticsAlgorithm::Ptr> &anal,
                            QObject *parent = nullptr) noexcept;
    void refresh() noexcept;
    void reloadSettings() noexcept;
public:
    inline ITradeAlgorithm::Ptr algo() const noexcept
    {
        return m_algo;
    }
    inline QSharedPointer <QSortFilterProxyModel> getTradingPairsProxyModel() const noexcept
    {
        return m_tradingPairsProxyModel;
    }
    MarketsModel const *getMarketsModel() const noexcept;
    MarketOrdersModel const *getMarketOrdersModel() const noexcept;
    TradeHistoryModel const *getTradeHistoryModel() const noexcept;
private:
    inline Exchange::Type getType() const noexcept
    {
        return m_algo.get()->api()->type();
    }
    inline IExchangeAPI * getApi() const noexcept
    {
        return m_algo.get()->api().get();
    }
    inline QSortFilterProxyModel *marketOrdersProxyModel() const noexcept
    {
        return m_marketOrdersProxyModel.get();
    }

    inline QSortFilterProxyModel *buyOrdersProxyModel() const noexcept
    {
        return m_buyOrdersProxyModel.get();
    }

    inline QSortFilterProxyModel *sellOrdersProxyModel() const noexcept
    {
        return m_sellOrdersProxyModel.get();
    }

    inline QSortFilterProxyModel *spreadMarketsProxyModel() const noexcept
    {
        return m_spreadMarketsProxyModel.get();
    }

    inline QSortFilterProxyModel *volumeMarketsProxyModel() const noexcept
    {
        return m_volumeMarketsProxyModel.get();
    }

    inline QSortFilterProxyModel *tradingPairsProxyModel() const noexcept
    {
        return m_tradingPairsProxyModel.get();
    }

    inline QSortFilterProxyModel *openOrdersProxyModel() const noexcept
    {
        return m_openOrdersProxyModel.get();
    }

    inline QSortFilterProxyModel *balancesProxyModel() const noexcept
    {
        return m_balancesProxyModel.get();
    }

    inline QSortFilterProxyModel *tradeHistoryProxyModel() const noexcept
    {
        return m_tradeHistoryProxyModel.get();
    }

    inline QSortFilterProxyModel *tradeHistorySelectedPairProxyModel() const noexcept
    {
        return m_tradeHistorySelectedPairProxyModel.get();
    }
private:
    using PrivateDataProxyModelPtr = QSharedPointer <QSortFilterProxyModel>;
    using PrivateDataDomainModelPtr = QSharedPointer <QAbstractItemModel>;
    const ITradeAlgorithm::Ptr   m_algo;
    const QList<AbstractAnalyticsAlgorithm::Ptr>     m_analyticsPool;
    PrivateDataDomainModelPtr m_marketsModel;
    PrivateDataDomainModelPtr m_marketOrdersModel;
    PrivateDataDomainModelPtr m_openOrdersModel;
    PrivateDataDomainModelPtr m_balancesModel;
    PrivateDataDomainModelPtr m_tradeHistoryModel;

    PrivateDataProxyModelPtr m_marketOrdersProxyModel;
    PrivateDataProxyModelPtr m_buyOrdersProxyModel;
    PrivateDataProxyModelPtr m_sellOrdersProxyModel;
    PrivateDataProxyModelPtr m_spreadMarketsProxyModel;
    PrivateDataProxyModelPtr m_volumeMarketsProxyModel;
    PrivateDataProxyModelPtr m_tradingPairsProxyModel;
    PrivateDataProxyModelPtr m_openOrdersProxyModel;
    PrivateDataProxyModelPtr m_balancesProxyModel;
    PrivateDataProxyModelPtr m_tradeHistoryProxyModel;
    PrivateDataProxyModelPtr m_tradeHistorySelectedPairProxyModel;
};

class ExchangePool final : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role
    {
        Exchange = Qt::UserRole + 1
    };

public:
    explicit ExchangePool(QObject *parent = nullptr) noexcept;
    ~ExchangePool() noexcept override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const noexcept override;
    QVariant data(const QModelIndex &index, int role = Qt::UserRole) const noexcept override;
    QHash< int, QByteArray > roleNames() const noexcept override;

    Q_INVOKABLE void append(const Exchange::Type type) noexcept;
    Q_INVOKABLE void remove(const Exchange::Type type, bool doSaveState = true) noexcept;
    Q_INVOKABLE void refreshModels() noexcept;
signals:
    void addMarket(const IExchangeApiController::MarketsGroup &group);
private slots:
    void loadState() noexcept;
    void saveState(const Exchange::Type type) noexcept;
private:
    void createEntry( const Exchange::Type type ) noexcept;
    void removeEntry( const ExchangeEntry::Storage::iterator &it ) noexcept;
    void reloadSettings() noexcept;
    ExchangeEntry::Storage::iterator findByType(const Exchange::Type type);
private:
    ExchangeEntry::Storage m_storage{};
};

#endif // EXCHANGE_POOL_H
