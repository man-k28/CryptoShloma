#ifndef STEP_H
#define STEP_H
#include "i_step.h"

class Step final : public IStep
{
public:
    Step(const qreal &declinePerc,
         const qreal &getGrowth,
         Hanukkah * const parentAlgo,
         const Market::ConstPtr &market,
         bool buyBlocked = false,
         bool sellBlocked = false) noexcept;
    void executeBuyLogic() noexcept override;
    void executeSellLogic() noexcept override;
private:
    qreal calculatePrevVolume(const Order::ConstContainer &container) noexcept;
};

#endif // STEP_H
