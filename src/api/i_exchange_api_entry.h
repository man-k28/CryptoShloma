#ifndef I_EXCHANGE_API_ENTRY_H
#define I_EXCHANGE_API_ENTRY_H
#include <base/order.h>
#include <base/balance.h>

namespace exchange {
struct MarketEntry
{
    Currency::CurrencyId tradeCurrencyId{};
    Currency::CurrencyId baseCurrencyId{};
    Market::Config params{};
};
struct OrderEntry
{
    Market::MarketId marketId{};
    Order::Config params{};
};

struct BalanceEntry
{
    Currency::CurrencyId currencyId{};
    Balance::Config params{};
};

using MarketEntryPtr = QSharedPointer<MarketEntry>;
using OrderEntryPtr = QSharedPointer<OrderEntry>;
using BalanceEntryPtr = QSharedPointer<BalanceEntry>;
}
#endif // I_EXCHANGE_API_ENTRY_H
