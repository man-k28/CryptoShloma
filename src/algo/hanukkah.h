#ifndef HANUKKAH_H
#define HANUKKAH_H
#include "i_trade_algo.h"
#include "hanukkah/i_step.h"
#include <QMap>
#include <QJsonArray>

class Hanukkah;
#pragma pack (push, 1)
class Hanukkah final: public ITradeAlgorithm
{
    Q_OBJECT
public:
    explicit Hanukkah(QObject *parent = nullptr) noexcept;
    ~Hanukkah() noexcept override;
    void unlockAllSteps(const Market::MarketId &id) noexcept;
private slots:
    bool start() noexcept override final;
    bool stop() noexcept override final;
    bool restart() noexcept override final;
    void reconfigure() noexcept override final;
    void timerEvent(QTimerEvent *event) noexcept override final;
    void removeMarket(const Market::MarketId &id) noexcept override final;
private:
    void execLogic() noexcept;
private:
    class StepGenerator
    {
    public:
        explicit StepGenerator() = default;
        explicit StepGenerator(const QJsonArray &data, Hanukkah * const parentAlgo, const Market::ConstPtr &market) noexcept;
        explicit StepGenerator(const StepGenerator &o) noexcept;
        StepGenerator& operator=(const StepGenerator &o) noexcept;
        bool update(const QJsonArray &data) noexcept;
        QJsonArray metaConfig = {};
        IStep::StepContainer container = {};
        Market::ConstPtr market() const;
    private:
        void setup() noexcept;
        Hanukkah * m_parentAlgo = nullptr;
        Market::ConstPtr m_market = {};
    };

    using SessionContainer = QMap<Market::MarketId, StepGenerator>;
    SessionContainer m_sessionContainer = {};
    qint32 m_timerId = 0;
};
#pragma pack (pop)

#endif // HANUKKAH_H
