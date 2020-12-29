#ifndef WEBCLIENT_H
#define WEBCLIENT_H
#include <QNetworkAccessManager>

class QSslError;
class QAuthenticator;
class QNetworkReply;
class QNetworkSession;

class WebClient final : public QObject
{
    Q_OBJECT
public:
    explicit WebClient      (QObject *parent = nullptr) noexcept;
    explicit WebClient(const WebClient &o) noexcept = delete;
    WebClient& operator=(const WebClient &o) noexcept = delete;
    ~WebClient() override;
    QNetworkReply * head(const QNetworkRequest &request) noexcept;
    QNetworkReply * get(const QNetworkRequest &request) noexcept;
    QNetworkReply * post(const QNetworkRequest &request, const QByteArray &data) noexcept;
    QNetworkReply * put(const QNetworkRequest &request, const QByteArray &data) noexcept;
    QNetworkReply * del(const QNetworkRequest &request) noexcept;
    inline quint32 requestCount() const noexcept {
        return m_requestCount;
    }
private slots:
    void slotAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator) noexcept;
#ifndef QT_NO_SSL
    void sslErrors(QNetworkReply *, const QList<QSslError> &errors) noexcept;
#endif
    void timerEvent(QTimerEvent *event) noexcept final;
private:
    inline void incRequestCount() noexcept {
        m_requestCount++;
    }
    bool isOnline();
    QNetworkAccessManager m_NetworkAccessManager{};
    QNetworkSession *m_session = nullptr;
    quint32 m_requestCount{0};
    qint32 m_GlobalRequestCountTimerId{0};
};

#endif // WEBCLIENT_H
