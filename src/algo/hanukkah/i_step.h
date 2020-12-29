#ifndef I_STEP_H
#define I_STEP_H
#include <boilerplate/singly_linked_stack.h>
#include <base/order.h>

class Hanukkah;
class IStep;
using StepPtr = QSharedPointer<IStep>;

#pragma pack (push, 1)
class IStep : public SinglyLinkedElement<StepPtr>
{
public:
    using StepContainer = SinglyLinkedStack<StepPtr>;
    explicit IStep(const qreal &declinePerc,
                   const qreal &growth,
                   Hanukkah * const parentAlgo,
                   const Market::ConstPtr &market,
                   bool buyBlocked,
                   bool sellBlocked) noexcept;
    IStep(const IStep &other) noexcept = delete;
    IStep & operator=(const IStep &other) noexcept = delete;

    virtual ~IStep();

    inline qreal getDeclinePerc() const noexcept
    {
        return m_declinePerc;
    }

    inline qreal getGrowth() const noexcept
    {
        return m_growth;
    }
    virtual void executeBuyLogic() = 0;
    virtual void executeSellLogic() = 0;
    inline bool isBuyBlocked() const noexcept
    {
        return m_isBuyBlocked;
    }
    inline bool isSellBlocked() const noexcept
    {
        return m_isSellBlocked;
    }

    void setBuyBlocked(bool isBlocked) noexcept;
    void setSellBlocked(bool isBlocked) noexcept;
protected:
    struct AverageData {
        qreal baseVolume{0.};
        qreal price{0.};
        qreal fee{0.};
        qreal volume{0.};
        quint16 ordersCount{0};
    };
    static AverageData calculateAverate(const Order::ConstContainer &container,
                                        const QDateTime &cycleTimeStart) noexcept;
    static qreal calculateStopLoss(const qreal &price,
                                   const qreal &oldFee,
                                   const qreal &baseVolume,
                                   const qreal &fee,
                                   const qreal &volume,
                                   const qreal &profit);
    void unlock() noexcept;
    void updateMarketMetaConfig(bool isBuyCompleted, const QDateTime &cycleTimeStamp = QDateTime());
protected:
    Hanukkah * const m_parentAlgo = nullptr;
    Market::ConstPtr m_market = {};
private:
    qreal m_declinePerc = 0.;
    qreal m_growth = 0.;
    bool m_isBuyBlocked = false;
    bool m_isSellBlocked = false;
};
#pragma pack (pop)
#endif // I_STEP_H
