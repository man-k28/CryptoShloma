#ifndef MARKETSMODEL_H
#define MARKETSMODEL_H
#include <QAbstractListModel>
#include <api/i_exchange_api_entry.h>

class MarketsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum MarketsModelRoles {
        MarketRole = Qt::UserRole + 1,
        IsTradableRole
    };

    explicit MarketsModel(QObject *parent = nullptr) noexcept;
    const Market::ConstPtr findMarketById(const Market::MarketId &id) const noexcept;
    const Currency::ConstPtr findCurrencyById(const Currency::CurrencyId &id) const noexcept;
private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const noexcept override;
    QVariant data(const QModelIndex &index, int role = Qt::UserRole) const noexcept override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) noexcept override;
    QHash<int, QByteArray> roleNames() const noexcept override;
public slots:
    void mergeCurrencies(const QList<Currency::Config> &data) noexcept;
    void mergeMarketInfo(const QList<exchange::MarketEntry> &data) noexcept;
    void mergeMarketData(const QList<exchange::MarketEntry> &data) noexcept;
    void refresh() noexcept;
    void updateMarketConfig(const Market::MarketId &marketId, const Market::UserConfig &config) noexcept;
signals:
    void removeTradePairIdFromTrading(const Market::MarketId &id);
    void mergeCompeleted();
private slots:
    void cancelTradingMarket(const Market::MarketId &id) noexcept;
private:
    Market::Container m_Data{};
    Currency::Container m_Currencys{};
};
#endif // MARKETSMODEL_H
