#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QWebSocket>
#include <QNetworkSession>

class QElapsedTimer;
class QSslError;
class QNetworkSession;
class QTimer;

class WebSocketClient final: public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClient(QObject *parent = nullptr);
    explicit WebSocketClient(const WebSocketClient &o) noexcept = delete;
    WebSocketClient& operator=(const WebSocketClient &o) noexcept = delete;
    ~WebSocketClient();
public slots:
    void open();
    void close();
    void ping();
public:
    void setBaseTemplate(const QString &baseTempl) noexcept;
    void addStreamTemplate(const QString &templ) noexcept;
    void removeStreamTemplate(const QString &templ, bool tryReconnect = true) noexcept;

    const QWebSocket *get() noexcept;
    bool isConnected() const noexcept;
    bool isOpen() const noexcept;
    const QNetworkSession * session() noexcept;
private slots:
    void onConnected();
    void onDisconnected();
    void onPong(quint64 elapsedTime, const QByteArray &payload);
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &message);
//    void onBinaryFrameReceived(const QByteArray &message);
//    void onTextFrameReceived(const QString &frame, bool isLastFrame);
    void onError(QAbstractSocket::SocketError error);
private slots:
    void onNetworkSessionOpened();
    void onNetworkSessionClosed();
    void onNetworkSessionError(QNetworkSession::SessionError error);
#ifndef QT_NO_SSL
    void onSslErrors(const QList<QSslError> &errors);
#endif
private:
    bool isOnline();
    void reconnect() noexcept;
private:
    QWebSocket *m_WebSocket;
    QThread *m_thread;
    QUrl m_url{};
    QNetworkSession *m_session = nullptr;
    QStringList m_streamTemplates{};
    QString m_baseWebSocketTemplate{};
    QTimer *m_pongTimer = nullptr;
};

#endif // WEBSOCKETCLIENT_H
