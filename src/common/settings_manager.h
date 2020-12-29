#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QSettings>
#include <QMetaEnum>
#include <QCoreApplication>
#include <QStandardPaths>
#include "constants.h"

class QQmlEngine;
class QJSEngine;

class SettingsManager final: public QObject
{
    Q_OBJECT

    static SettingsManager *m_SettingsManager;
    explicit SettingsManager(QObject *parent = nullptr) noexcept;
    SettingsManager(const SettingsManager &) noexcept = delete;
    SettingsManager &operator = (const SettingsManager &) noexcept = delete;
public:
    enum Section {
        General,
        Hanukkah,
        Binance
    };
    Q_ENUM(Section)

    enum Key {
        TradeBaseSumm,
        MinSpread,
        MinProfit,
        MinBaseVolume,
        MinCoinPrice,
        TradeDifference, //NOTE: нужно для того, что бы выставлять +- от лучших ордеров по рынку
        HanukkahTable,
        HanukkahMaxSteps,
        BinanceEnabled,
        BinanceAPIKey,
        BinanceSecretKey,
        EnableTestData
    };
    Q_ENUM(Key)

    static void setDefaults(const QString &str) noexcept;
    Q_INVOKABLE static QVariant get(const Key k, const Section s = General) noexcept;
    Q_INVOKABLE static void set(const Key k, const Section s = General, const QVariant &data = QVariant()) noexcept;
    //Метод с макросом Q_INVOKABLE, возвращающий кастомный тип не работает, должен возвращаться QVariant!
    //static ValueRef set(Key, Section s = General);
#ifdef QT_QML_LIB
    static QObject *qobject_settingsmanager_provider(QQmlEngine *engine, QJSEngine *scriptEngine) noexcept;
#endif
    static SettingsManager *getInstance() noexcept;
    static void drop() noexcept;
private:

    QString keyPath(const Section s, const Key k) noexcept;
    QMetaEnum m_Keys{};
    QMetaEnum m_Sections{};
    QMap<QString, QVariant> defaults{};
    QMap<QString, QVariant> conf_cache{};
#ifndef QT_NO_DEBUG
    QSettings conf {qApp->applicationDirPath() + "/config.ini", QSettings::IniFormat};
#else
    QSettings conf {QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QString("/.%1/config.ini").arg(GLOBAL_APP_NAME), QSettings::IniFormat};
#endif
};

#endif // SETTINGSMANAGER_H
