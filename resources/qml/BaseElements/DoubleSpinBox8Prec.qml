import QtQuick 2.12
import QtQuick.Controls 2.12

SpinBox {
    id: spinbox
    from: 0
    value: 0
    to: 100000000
    stepSize: 1
    editable: true

    property int decimals: 8
    property real realValue: value / 100000000

    validator: DoubleValidator {
        bottom: Math.min(spinbox.from, spinbox.to)
        top:  Math.max(spinbox.from, spinbox.to)
    }

    textFromValue: function(value, locale) {
        return Number(value / 100000000).toFixed(spinbox.decimals)
    }

    valueFromText: function(text, locale) {
        return Number.fromLocaleString(locale, text) * 100000000
    }
}
