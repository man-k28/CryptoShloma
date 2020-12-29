#ifndef ENUMS_H
#define ENUMS_H
#include <qobjectdefs.h>

class Exchange final {
    Q_GADGET
public:
    Exchange() = delete;
    enum class Type : quint8
    {
        Unknown = 1,
        Binance
    };
    Q_ENUM(Type)
};

enum AlgoType {
    HanukkahAlgo,
    SprederAlgo
};

enum AnalyticsAlgoType {
    TrendSearcherAlgo
};

#endif // ENUMS_H
