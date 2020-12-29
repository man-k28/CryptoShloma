#include "crud.h"
#include <QWriteLocker>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlRecord>
#include <QCoreApplication>
#include <QStandardPaths>
#include <Logger.h>
#include <common/constants.h>
#include <include/version.hpp>

Crud *Crud::m_Crud = nullptr;

Crud::Crud(QObject *parent) noexcept
    : QObject(parent)
    , m_Database(QSqlDatabase::addDatabase(QStringLiteral("QSQLITE")))
{
}

Crud::~Crud() noexcept
{
    m_Database.close();
    QSqlDatabase::removeDatabase(QStringLiteral("defaultConnection"));
}

Crud *Crud::getInstance()
{
    if ( !m_Crud ) {
        static QReadWriteLock mutex;
        QWriteLocker locker(&mutex);
        m_Crud = new Crud;
    }
    return m_Crud;
}

void Crud::drop()
{
    if ( m_Crud ) {
        static QReadWriteLock mutex;
        QWriteLocker locker(&mutex);
        m_Crud->closeDatabase();
        delete m_Crud;
        m_Crud = nullptr;
    }
}

void Crud::openDatabase()
{
    QWriteLocker locker(&mutex);
    QFileInfo info{QStringLiteral(":/common/share/sql/dbCryptoShloma.sqlite")};

#ifndef QT_NO_DEBUG
    const QString dbName {QCoreApplication::applicationDirPath() + QString("/%1").arg(info.fileName())};
#else
    const QString dbName {QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QString("/.%1/%2").arg(GLOBAL_APP_NAME).arg(info.fileName())};
#endif
    m_Database.setDatabaseName(dbName);

    if ( !QFile::exists(dbName) ) {
        if ( !QFile::copy(info.absoluteFilePath(), dbName) ) {
            LOG_WARNING(QStringLiteral("Error of create database file!"));
            return;
        }
        if ( !QFile::setPermissions(dbName, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::ReadGroup | QFileDevice::ReadOther) ) {
            LOG_WARNING(QStringLiteral("Error of set permissions of database file!"));
            return;
        }
        if ( !m_Database.open() ) {
            LOG_WARNING(m_Database.lastError().text());
            m_isOpen = false;
            emit isOpenChanged();
            return;
        }
    } else {
        if ( !m_Database.open() ) {
            LOG_WARNING(m_Database.lastError().text());
            m_isOpen = false;
            emit isOpenChanged();
            return;
        } else {
            QSqlQuery result;
            if ( !result.exec(QStringLiteral("SELECT value FROM variables WHERE key = 'scheme_version' LIMIT 1")) ) {
                LOG_ERROR(QString(QStringLiteral("%1. Query value: '%2'")).arg(result.lastError().text()).arg(result.lastQuery()));
                return;
            }

            if ( result.first() ) {
                const auto &ver = result.value(0).toString();
                const auto &tradeBotVer = cryptoShlomaDatabaseVersionStr();
                if ( tradeBotVer != ver ) {
                    m_Database.close();
                    QFile::remove(dbName);
                    if ( !QFile::copy(info.absoluteFilePath(), dbName) ) {
                        LOG_WARNING(QStringLiteral("Error of create database file!"));
                        return;
                    }
                    if ( !QFile::setPermissions(dbName, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::ReadGroup | QFileDevice::ReadOther) ) {
                        LOG_WARNING(QStringLiteral("Error of set permissions of database file!"));
                        return;
                    }

                    if ( !m_Database.open() ) {
                        LOG_WARNING(m_Database.lastError().text());
                        m_isOpen = false;
                        emit isOpenChanged();
                        return;
                    } else {
                        QSqlQuery query;

                        if ( !query.exec(QString("INSERT OR REPLACE INTO variables VALUES ('scheme_version', '%1')").arg(tradeBotVer)) )
                            LOG_ERROR(QString(QStringLiteral("%1. Query value: '%2'")).arg(query.lastError().text()).arg(query.lastQuery()));
                    }
                }
            }
        }
    }

    m_isOpen = true;
    emit isOpenChanged();
}

void Crud::closeDatabase()
{
    if ( getIsOpen() ) {
        m_Database.close();
        m_isOpen = false;
        emit isOpenChanged();
    }
}

bool Crud::getIsOpen() const
{
    return m_isOpen;
}

QSqlQuery Crud::executeQuery(const QString &sql)
{
    QWriteLocker locker(&mutex);
    QSqlQuery query;

    if ( !query.exec(sql) )
        LOG_ERROR(QString(QStringLiteral("%1. Query value: '%2'")).arg(query.lastError().text()).arg(query.lastQuery()));
    return query;
}
