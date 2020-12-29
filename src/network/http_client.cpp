#include "network/http_client.h"
#include <QNetworkReply>
#include <QAuthenticator>
#include <QNetworkSession>
#include <QTimerEvent>
#include <Logger.h>

WebClient::WebClient(QObject *parent) noexcept
    : QObject(parent)
{
    m_session = new QNetworkSession(m_NetworkAccessManager.activeConfiguration(), this);
    m_session->open();
    m_session->waitForOpened(3000);
    connect(&m_NetworkAccessManager, &QNetworkAccessManager::authenticationRequired,
            this, &WebClient::slotAuthenticationRequired);
#ifndef QT_NO_SSL
    connect(&m_NetworkAccessManager, &QNetworkAccessManager::sslErrors,
            this, &WebClient::sslErrors);
#endif
    connect(&m_NetworkAccessManager, &QNetworkAccessManager::sslErrors,
            this, &WebClient::sslErrors);
    m_GlobalRequestCountTimerId = startTimer(60000);
}

WebClient::~WebClient()
{
    if ( m_GlobalRequestCountTimerId) {
        killTimer(m_GlobalRequestCountTimerId);
        m_GlobalRequestCountTimerId = 0;
    }
}

QNetworkReply *WebClient::head(const QNetworkRequest &request) noexcept
{
    incRequestCount();
    return isOnline() ? m_NetworkAccessManager.head(request) : nullptr;
}

QNetworkReply * WebClient::get(const QNetworkRequest &request) noexcept
{
    incRequestCount();
    return isOnline() ? m_NetworkAccessManager.get(request) : nullptr;
}

QNetworkReply *WebClient::post(const QNetworkRequest &request, const QByteArray &data) noexcept
{
    incRequestCount();
    return isOnline() ? m_NetworkAccessManager.post(request, data) : nullptr;
}

QNetworkReply *WebClient::put(const QNetworkRequest &request, const QByteArray &data) noexcept
{
    incRequestCount();
    return isOnline() ? m_NetworkAccessManager.put(request, data) : nullptr;
}

QNetworkReply *WebClient::del(const QNetworkRequest &request) noexcept
{
    incRequestCount();
    return isOnline() ? m_NetworkAccessManager.deleteResource(request) : nullptr;
}

void WebClient::slotAuthenticationRequired(QNetworkReply *, QAuthenticator *) noexcept
{

}

#ifndef QT_NO_SSL
void WebClient::sslErrors(QNetworkReply *, const QList<QSslError> &errors) noexcept
{
    QString errorString;
    for (const QSslError &error : errors) {
        if (!errorString.isEmpty())
            errorString += '\n';
        errorString += error.errorString();
    }
    LOG_ERROR(errorString);
//    if (QMessageBox::warning(this, tr("SSL Errors"),
//                             tr("One or more SSL errors has occurred:\n%1").arg(errorString),
//                             QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Ignore) {
//        reply->ignoreSslErrors();
    //    }
}

#endif

void WebClient::timerEvent(QTimerEvent *event) noexcept
{
    if ( event->timerId() == m_GlobalRequestCountTimerId ) {
        LOG_TRACE(QString(QStringLiteral("Requests count per minute %1")).arg(requestCount()));
        m_requestCount = 0;

        if ( !m_session->isOpen() ) {
            m_session->open();
            m_session->waitForOpened(3000);
        }
    }
}

bool WebClient::isOnline()
{
    if ( m_session->isOpen() )
        return true;
    else return false;
}
