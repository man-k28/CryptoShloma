#include "cryptoshloma.h"
#include <QQmlComponent>
#include <QQmlContext>
#include <Logger.h>
#include "common/settings_manager.h"
#include "crud.h"
#include "emulator.h"
#include <QSortFilterProxyModel>

CryptoShloma::CryptoShloma(QObject *parent) :
    QObject(parent)
{
    qmlRegistration();
    Crud::getInstance()->openDatabase();
}

CryptoShloma::~CryptoShloma() noexcept
{
    LOG_INFO("Shutdown");
}

bool CryptoShloma::init()
{
    LOG_INFO("Booting...");
    reloadSettings(true);

    m_Engine.rootContext()->setContextProperty(QStringLiteral("CryptoShloma"), this);
    m_Engine.rootContext()->setContextProperty(QStringLiteral("Crud"), Crud::getInstance());
    m_Engine.rootContext()->setContextProperty(QStringLiteral("ExchangePool"), &m_exchangePool);
    m_Engine.addImportPath(QStringLiteral("qrc:/ui/qml/resources/qml"));
    m_Engine.addImportPath(QStringLiteral("qrc:/ui/scripts/resources/qml"));
    QQmlComponent component(&m_Engine, QUrl(QStringLiteral("qrc:/ui/qml/resources/qml/main.qml")));
    component.create();
    if (component.isError()) {
        LOG_ERROR(component.errorString());
        return false;
    }

    return true;
}

void CryptoShloma::reloadSettings(const bool initial) noexcept
{
    if ( SettingsManager::get(SettingsManager::BinanceEnabled, SettingsManager::Binance).toBool() )
        m_exchangePool.append(Exchange::Type::Binance);
    else
        if ( !initial )
            m_exchangePool.remove(Exchange::Type::Binance);
}

void CryptoShloma::restartEmulation() noexcept
{
    m_exchangePool.remove(Exchange::Type::Binance, false);
}

void CryptoShloma::qmlRegistration() noexcept
{
    qmlRegisterSingletonType<SettingsManager>("CryptoShloma.Common", 1, 0, "SettingsManager", SettingsManager::qobject_settingsmanager_provider);

    qRegisterMetaType<QSortFilterProxyModel *>("QSortFilterProxyModel *");
    qRegisterMetaType<Emulator *>("Emulator *");
    qmlRegisterType<Order>("CryptoShloma.Core", 1, 0, "Order");
    qRegisterMetaType<Order::OrderId>("Order::OrderId");
    qRegisterMetaType<QList<Currency::Config> >("QList<Currency::Config>");
    qRegisterMetaType<Market::MarketId>("Market::MarketId");
    qRegisterMetaType<Market::UserConfig>("Market::UserConfig");
    qRegisterMetaType<QList<Order::OrderId> >("QList<Order::OrderId>");
    qRegisterMetaType<QList<exchange::MarketEntry> >("QList<exchange::MarketEntry>");
    qRegisterMetaType<QList<exchange::OrderEntry> >("QList<exchange::OrderEntry>");
    qRegisterMetaType<QList<exchange::BalanceEntry> >("QList<exchange::BalanceEntry>");
    qmlRegisterUncreatableType<Exchange>("CryptoShloma.Gui", 1, 0, "ExchangeType", "Not creatable as it is an enum type");
}
