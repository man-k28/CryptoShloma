#include "settings_manager.h"
#include <QReadWriteLock>
#include <QWriteLocker>
#include <QRegularExpression>
#include <common/encrypt.h>
#include <common/constants.h>

SettingsManager *SettingsManager::m_SettingsManager = nullptr;

SettingsManager::SettingsManager(QObject *parent) noexcept :
    QObject(parent)
{
    const QMetaObject &mo = staticMetaObject;
    int idx = mo.indexOfEnumerator("Key");
    m_Keys = mo.enumerator(idx);

    idx = mo.indexOfEnumerator("Section");
    m_Sections = mo.enumerator(idx);
}

QVariant SettingsManager::get(const Key k, const Section s) noexcept
{
    SettingsManager *self = getInstance();
    const QString &key = self->keyPath(s, k);

    const QVariant &cacheValue = self->conf_cache.value(key);

    if ( !cacheValue.isNull() )
        return cacheValue;
    if ( key.contains(QStringLiteral("key"), Qt::CaseInsensitive) )
        return Encrypt::decrypt(self->conf.value(key, self->defaults[key]).toByteArray(), GLOBAL_APP_NAME.toUtf8());
    else
        return self->conf.value(key, self->defaults[key]);
}

void SettingsManager::set(const Key k, const Section s, const QVariant& data) noexcept
{
    SettingsManager *self = getInstance();
    const QString &keyPath = self->keyPath(s, k);
    self->conf_cache[keyPath] = data;
    if ( keyPath.contains(QStringLiteral("key"), Qt::CaseInsensitive) )
        self->conf.setValue(keyPath, Encrypt::encrypt(data.toByteArray(), GLOBAL_APP_NAME.toUtf8()));
    else
        self->conf.setValue(keyPath, data);
}

#ifdef QT_QML_LIB
QObject *SettingsManager::qobject_settingsmanager_provider(QQmlEngine *engine, QJSEngine *scriptEngine) noexcept
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return getInstance();
}
#endif

void SettingsManager::setDefaults(const QString &str) noexcept
{
    SettingsManager *self = getInstance();

    //section/key : value
    //section - optional
    QRegExp rxRecord{"^\\s*(((\\w+)/)?(\\w+))\\s*:\\s*([^\\s].{0,})(\\B|\\b)\\s*$"};

    const QStringList &kvs = str.split(QRegularExpression(";\\W*")); //key-values
    for(const QString &kv : kvs) {
        if(rxRecord.indexIn(kv) != -1) {
            const QString &section = rxRecord.cap(3);
            const QString &key = rxRecord.cap(4);
            QString value = rxRecord.cap(5);

            const int iKey = self->m_Keys.keyToValue(key.toUtf8().data());
            if(iKey != -1) {
                const int iSection = self->m_Sections.keyToValue(section.toUtf8().data());
                if(section.isEmpty() || iSection != -1) {
                    self->defaults[rxRecord.cap(1)] = value.remove("\\");
                }
            }
        }
    }
    for ( const auto &key : self->conf.allKeys() ) {
        if ( key.contains(QStringLiteral("key"),Qt::CaseInsensitive) )
            self->conf_cache[key] = Encrypt::decrypt(self->conf.value(key, self->defaults[key]).toByteArray(), GLOBAL_APP_NAME.toUtf8());
        else
            self->conf_cache[key] = self->conf.value(key, self->defaults[key]);
    }
}

//PRIVATE METHODS--------------------------------------------------------------
QString SettingsManager::keyPath(const Section s, const Key k) noexcept
{
    const char *szSection = m_Sections.valueToKey(s);
    const char *szKey = m_Keys.valueToKey(k);
    return (s == General) ? QString("%1").arg(szKey) : QString("%2/%1").arg(szKey).arg(szSection);
}

SettingsManager *SettingsManager::getInstance() noexcept
{
    if ( !m_SettingsManager ) {
        static QReadWriteLock mutex;
        QWriteLocker locker(&mutex);
        m_SettingsManager = new SettingsManager;
    }
    return m_SettingsManager;
}

void SettingsManager::drop() noexcept
{
    static QReadWriteLock mutex;
    QWriteLocker locker(&mutex);
    delete m_SettingsManager;
    m_SettingsManager = nullptr;
}
