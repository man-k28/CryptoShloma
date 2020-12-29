#pragma once
#include <QString>
static const QString CONST_BTC{QStringLiteral("BTC")};
static const qint32 CONST_BTC_ID{1};
static const qint32 CONST_MIN_TIMEOUT{1000};
static const qreal PRECISSION{0.00000001};
static const qint32 CONST_ORDERS_COUNT{5};
static const qreal BINANCE_FEE{0.00075}; //NOTE: комиссия по BNB

static const QString GLOBAL_APP_NAME{QStringLiteral("CryptoShloma")};
#define DOUBLE_TO_STR(val) QString::number(val, 'f', 8)
