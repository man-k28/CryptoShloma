#ifndef BALANCE_H
#define BALANCE_H
#include <QDateTime>
#include "currency.h"

class Balance;
#pragma pack (push, 1)
class Balance final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Currency *currency READ getCurrencyPtr CONSTANT) //TODO зарегистрировать в qml
    Q_PROPERTY(QString statusMessage READ getStatusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(qreal total READ getTotal NOTIFY totalChanged)
    Q_PROPERTY(qreal available READ getAvailable NOTIFY availableChanged)
    Q_PROPERTY(qreal heldForTrades READ getHeldForTrades NOTIFY heldForTradesChanged)
    Q_PROPERTY(BalanceStatus status READ getStatus NOTIFY statusChanged) //TODO зарегистрировать в qml
public:
    enum BalanceStatus {
        Ok = 1,
        Maintenance
    };
    Q_ENUM(BalanceStatus)

    using ConstPtr = QSharedPointer<const Balance>;
    using Ptr = QSharedPointer<Balance>;
public:
    struct Config
    {
        QDateTime       timestamp{};
        QString         statusMessage{};
        qreal           total{0.};
        qreal           available{0.};
        qreal           heldForTrades{0.};
        BalanceStatus   status{Ok};
    };
public:
    explicit Balance(QObject *parent = nullptr);
    explicit Balance(const Config &config);
    Balance(const Balance &o) noexcept;
    Balance(Balance &&o) noexcept;

    const Currency::ConstPtr getCurrency() const noexcept;
    void setCurrency(const Currency::ConstPtr &value) noexcept;

    const QString &getStatusMessage() const noexcept;
    void setStatusMessage(const QString &value) noexcept;

    const qreal &getTotal() const noexcept;
    void setTotal(const qreal &value) noexcept;

    const qreal &getAvailable() const noexcept;
    void setAvailable(const qreal &value) noexcept;

    const qreal &getHeldForTrades() const noexcept;
    void setHeldForTrades(const qreal &value) noexcept;

    BalanceStatus getStatus() const noexcept;
    void setStatus(const BalanceStatus value) noexcept;

    const QDateTime &getTimestamp() const noexcept;
    void setTimestamp(const QDateTime &value) noexcept;

    void setConfig(const Config &config) noexcept;
signals:
    void statusMessageChanged();
    void totalChanged();
    void availableChanged();
    void heldForTradesChanged();
    void statusChanged();
private:
    Currency *getCurrencyPtr() const noexcept;
private:
    Currency::ConstPtr      m_currency{};
    Config                  m_config{};
};
#pragma pack (pop)
#endif // BALANCE_H
