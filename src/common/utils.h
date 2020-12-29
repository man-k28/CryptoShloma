#ifndef UTILS_H
#define UTILS_H
#include <QJsonValue>
#include <math.h>

namespace CryptoShloma {
namespace Utils {
    inline QString toDoublePrec8(const qreal &val) noexcept
    {
        return QString::number(val, 'f', 8);
    }

    inline QString toULongLong(const qreal &val) noexcept //TODO: deprecated
    {
        return QString::number(static_cast<quint64>(val));
    }

    inline QString toDoublePrec1(const qreal &val) noexcept //TODO: deprecated
    {
        return QString::number(val, 'f', 1);
    }

    inline qreal toDouble(const QJsonValue &val) noexcept
    {
        return val.toString().toDouble(); //FIXME: добавлено из-за того, что QJSonValue.toDouble() теряет точность при уже при 7 знаках, может баг Qt
    }

    inline qreal roundingFloor(const qreal &value, const qreal &rounding)
    {
        Q_ASSERT_X(!qFuzzyIsNull(value), "devide", "Devision by zero");
        const qreal devide = rounding / value;
        const qreal round = std::floor(devide);
        const qreal ret = round * value;
        return ret;
    }

    inline qreal roundingCeil(const qreal &value, const qreal &rounding)
    {
        Q_ASSERT_X(!qFuzzyIsNull(value), "devide", "Devision by zero");
        const qreal devide = rounding / value;
        const qreal round = std::ceil(devide);
        const qreal ret = round * value;
        return ret;
    }

    inline qreal rounding(const qreal &value, const qreal &rounding)
    {
        Q_ASSERT_X(!qFuzzyIsNull(value), "devide", "Devision by zero");
        qint64 round = static_cast<qint64>((rounding * std::pow(10,8)) / (value * std::pow(10,8)));
        return static_cast<qreal>(round )* value;
    }
}
}
#endif // UTILS_H
