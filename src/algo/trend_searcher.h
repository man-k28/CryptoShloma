#ifndef TRENDSEARCHER_H
#define TRENDSEARCHER_H
#include "i_analytics_algo.h"

class TrendSearcher final: public AbstractAnalyticsAlgorithm
{
    Q_OBJECT
public:
    explicit TrendSearcher(QObject *parent = nullptr) noexcept;
private slots:
    void timerEvent(QTimerEvent *event) noexcept override final;
    bool start() noexcept override final;
    bool stop() noexcept override final;
private:
    qreal calcPerc(const qreal &a, const qreal &b) const noexcept;
    qreal findFirstSellPrice(const Order::ConstContainer &model, const QDateTime &cycleTimeStart) noexcept;
    qint32  m_TimerId{0};
};

#endif // TRENDSEARCHER_H
