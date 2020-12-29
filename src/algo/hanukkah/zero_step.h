#ifndef ZEROSTEP_H
#define ZEROSTEP_H
#include "i_step.h"

class ZeroStep final : public IStep
{
public:
    ZeroStep(const qreal &declinePerc,
             const qreal &getGrowth,
             Hanukkah * const parentAlgo,
             const Market::ConstPtr &market,
             bool buyBlocked = false,
             bool sellBlocked = false) noexcept;
    void executeBuyLogic() noexcept override;
    void executeSellLogic() noexcept override;
};

#endif // ZEROSTEP_H
