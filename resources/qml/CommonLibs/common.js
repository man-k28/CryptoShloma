function toDouble(styledData) {
    if ( styledData !== undefined && styledData !== null) {
        if ( typeof styledData === 'number' ) {
            if ( (Number(styledData) === styledData && styledData % 1 === 0) )
                return styledData
            else
                return styledData.toFixed(8)
        } else
            return styledData

    } else
        return ""
}
