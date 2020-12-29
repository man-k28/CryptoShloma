#include "websocket_client.h"
#include <Logger.h>
#include <QThread>
#include <QTimer>
#include <QNetworkAccessManager>

WebSocketClient::WebSocketClient(QObject *parent)
    : QObject(parent)
    , m_WebSocket( new QWebSocket())
    , m_thread( new QThread)
    , m_pongTimer (new QTimer(this) )
{
    qRegisterMetaType<QWebSocketProtocol::CloseCode>("QWebSocketProtocol::CloseCode");
    QNetworkAccessManager manager;
    m_session = new QNetworkSession(manager.activeConfiguration(), this);

    connect(m_session, QOverload<QNetworkSession::SessionError>::of(&QNetworkSession::error), this, &WebSocketClient::onNetworkSessionError);
    connect(m_session, &QNetworkSession::opened, this, &WebSocketClient::onNetworkSessionOpened);
    connect(m_session, &QNetworkSession::closed, this, &WebSocketClient::onNetworkSessionClosed);

    m_session->open();
    m_session->waitForOpened(3000);

    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    connect(m_thread, &QThread::finished, m_WebSocket, &QObject::deleteLater);

    m_WebSocket->moveToThread(m_thread);
    m_thread->start();
    connect(m_WebSocket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(m_WebSocket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(m_WebSocket, &QWebSocket::pong, this, &WebSocketClient::onPong);
    connect(m_WebSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WebSocketClient::onError);
    connect(m_WebSocket, &QWebSocket::textMessageReceived,
            this, &WebSocketClient::onTextMessageReceived);
//    connect(m_WebSocket, &QWebSocket::textFrameReceived,
//            this, &WebSocketClient::onTextFrameReceived);
//    connect(m_WebSocket, &QWebSocket::binaryMessageReceived,
//            this, &WebSocketClient::onBinaryMessageReceived);
//    connect(&m_WebSocket, &QWebSocket::binaryFrameReceived,
//            this, &WebSocketClient::onBinaryFrameReceived);
#ifndef QT_NO_SSL
    connect(m_WebSocket, &QWebSocket::sslErrors,
            this, &WebSocketClient::onSslErrors);
#endif
    m_pongTimer->setSingleShot(true);
    connect(m_pongTimer, &QTimer::timeout, [this] () {
        reconnect();
    });
}

WebSocketClient::~WebSocketClient()
{
    m_thread->quit();
    m_thread->wait();
}

void WebSocketClient::open()
{
    if ( m_url.isValid() && !m_url.isEmpty() && isOnline() )
        QMetaObject::invokeMethod(m_WebSocket, "open", Qt::QueuedConnection, Q_ARG(QUrl, m_url));
//        m_WebSocket->open(m_url);
}

void WebSocketClient::setBaseTemplate(const QString &baseTempl) noexcept
{
    m_baseWebSocketTemplate = baseTempl;
}

void WebSocketClient::addStreamTemplate(const QString &templ) noexcept
{
    //FIXME: не надо делать переподлючение, у бинанса есть апи по подписке.
    if ( !m_streamTemplates.contains(templ) ) {
        m_streamTemplates << templ;
        m_url = m_baseWebSocketTemplate + m_streamTemplates.join("/");
        reconnect();
    }
}

void WebSocketClient::removeStreamTemplate(const QString &templ, bool tryReconnect) noexcept
{
    if ( m_streamTemplates.contains(templ) ) {
        m_streamTemplates.removeOne(templ);
        m_url = m_baseWebSocketTemplate + m_streamTemplates.join("/");
        if ( tryReconnect )
            reconnect();
        else {
            if ( isConnected() )
                close();
        }
    }
}

void WebSocketClient::close()
{
    QMetaObject::invokeMethod(m_WebSocket, "close", Qt::QueuedConnection,
                              Q_ARG(QWebSocketProtocol::CloseCode, QWebSocketProtocol::CloseCodeNormal),
                              Q_ARG(QString, QString("Reconnect by add new stream")));
//    m_WebSocket->close(QWebSocketProtocol::CloseCodeNormal, QString("Reconnect by add new stream"));
}

void WebSocketClient::ping()
{
    QMetaObject::invokeMethod(m_WebSocket, "ping", Qt::QueuedConnection);
//    m_WebSocket->ping();
    m_pongTimer->start(5000);
}

const QWebSocket *WebSocketClient::get() noexcept
{
    return m_WebSocket;
}

bool WebSocketClient::isConnected() const noexcept
{
    return (m_WebSocket->state() == QAbstractSocket::ConnectedState) || (m_WebSocket->state() == QAbstractSocket::ConnectingState);
}

bool WebSocketClient::isOpen() const noexcept
{
    return m_WebSocket->state() == QAbstractSocket::ConnectedState;
}

const QNetworkSession *WebSocketClient::session() noexcept
{
    return m_session;
}

void WebSocketClient::onConnected()
{
    LOG_INFO("Websocket connected to: " + m_WebSocket->request().url().toString());
}

void WebSocketClient::onDisconnected()
{
    LOG_INFO(QString("Websocket disconnected from: %1").arg(m_WebSocket->request().url().toString()));
}

void WebSocketClient::onPong(quint64 elapsedTime, const QByteArray &payload)
{
    Q_UNUSED(payload);
    LOG_TRACE(QString("Websocket receive pong message from: %1, elapsed: %2 ms")
              .arg(m_WebSocket->request().url().toString())
              .arg(elapsedTime));
    m_pongTimer->stop();
}

void WebSocketClient::onTextMessageReceived(const QString &message)
{
    LOG_TRACE(QString("get bytes: %1").arg(message.size()));
    LOG_TRACE(message);
}

void WebSocketClient::onBinaryMessageReceived(const QByteArray &message)
{
    LOG_TRACE(QString("get bytes: %1").arg(message.size()));
    LOG_TRACE("%s", message.constData());
}

//void WebSocketClient::onBinaryFrameReceived(const QByteArray &message)
//{
//    LOG_DEBUG(QString("get bytes: %1").arg(message.size()));
//    LOG_TRACE("%s", message.constData());
//}

//void WebSocketClient::onTextFrameReceived(const QString &frame, bool isLastFrame)
//{
//    LOG_DEBUG(QString("get bytes: %1").arg(frame.size()));
//    LOG_TRACE(frame);
//}

void WebSocketClient::onError(QAbstractSocket::SocketError error)
{
    LOG_ERROR(QString("Code: %1 error: %2").arg(error).arg(m_WebSocket->errorString()));
    m_pongTimer->start(1000);
}

void WebSocketClient::onNetworkSessionOpened()
{
    LOG_TRACE(QString("NetworkSession opened: %1 address: %2")
             .arg(m_session->interface().name())
             .arg(m_session->interface().hardwareAddress()));
    reconnect();
}

void WebSocketClient::onNetworkSessionClosed()
{
    LOG_TRACE(QString("NetworkSession closed: %1 address %2")
             .arg(m_session->interface().name())
             .arg(m_session->interface().hardwareAddress()));
}

void WebSocketClient::onNetworkSessionError(QNetworkSession::SessionError error)
{
    LOG_ERROR(QString("NetworkSession code: %1 error: %2")
              .arg(error)
              .arg(m_session->errorString()));
}

#ifndef QT_NO_SSL
void WebSocketClient::onSslErrors(const QList<QSslError> &errors)
{
    QString errorString;
    for (const QSslError &error : errors) {
        if (!errorString.isEmpty())
            errorString += '\n';
        errorString += error.errorString();
    }
    LOG_ERROR(errorString);
}
#endif

bool WebSocketClient::isOnline()
{
    if ( m_session->isOpen() )
        return true;
    else return false;
}

void WebSocketClient::reconnect() noexcept
{
    if ( isConnected() )
        close();
    open();
}

