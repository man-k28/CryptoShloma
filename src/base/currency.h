#ifndef CURRENCY_H
#define CURRENCY_H
#include <boilerplate/base_entry.h>

class Currency;

#pragma pack (push, 1)
class Currency final : public QObject, public Boilerplate::BaseEntry<Currency, QString>
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
    Q_PROPERTY(QString symbol READ getSymbol NOTIFY idChanged)
    Q_PROPERTY(QString statusMessage READ getStatusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(CurrencyId id READ getId NOTIFY idChanged) //TODO зарегистрировать в qml
    Q_PROPERTY(CurrencyStatusListing listingStatus READ getListingStatus NOTIFY listingStatusChanged) //TODO зарегистрировать в qml
    Q_PROPERTY(CurrencyStatus status READ getStatus NOTIFY statusChanged) //TODO зарегистрировать в qml

public:
    enum CurrencyStatusListing {
        Active = 1,
        Delisting
    };
    Q_ENUM(CurrencyStatusListing)

    enum CurrencyStatus {
        Ok = 1,
        Maintenance,
        NoConnections,
        Offline
    };
    Q_ENUM(CurrencyStatus)

    using CurrencyId = Id;
public:
    struct Config
    {
        CurrencyId              id{};
        QString                 name{};
        QString                 statusMessage{};
        CurrencyStatusListing   listingStatus{Active};
        CurrencyStatus          status{Ok};
    };
public:
    explicit Currency(QObject *parent = nullptr);
    explicit Currency(const Config &config);
    Currency(const Currency &other);
    Currency(Currency &&o) noexcept;

    const QString &getName() const noexcept;
    void setName(const QString &value) noexcept;

    const QString &getSymbol() const noexcept;

    const QString &getStatusMessage() const noexcept;
    void setStatusMessage(const QString &value) noexcept;

    const CurrencyId &getId() const noexcept;
    void setId(const CurrencyId &value) noexcept;

    CurrencyStatusListing getListingStatus() const noexcept;
    void setListingStatus(const CurrencyStatusListing value) noexcept;

    CurrencyStatus getStatus() const noexcept;
    void setStatus(const CurrencyStatus value) noexcept;

    void setConfig(const Config &config) noexcept;
signals:
    void nameChanged();
    void symbolChanged();
    void statusMessageChanged();
    void idChanged();
    void listingStatusChanged();
    void statusChanged();
private:
    Config                  m_config{};
};
#pragma pack (pop)
#endif // CURRENCY_H
