#ifndef BASE_ENTRY_H
#define BASE_ENTRY_H
#include "strong_typedef.h"

namespace Boilerplate {

template<
    typename TYPE,
    typename ID
>
class BaseEntry
{
public:
    virtual ~BaseEntry() = default;
    using ConstPtr = QSharedPointer<const TYPE>;
    using Ptr = QSharedPointer<TYPE>;
    using Container = QList<Ptr>;
    using ConstContainer = QList<ConstPtr>;
    using Id = Boilerplate::StrongTypedef<ID>;
};
}

#endif // BASE_ENTRY_H
