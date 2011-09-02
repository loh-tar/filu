/*
 *  Purpose:
 *     To fetch all symbols a FI has ordered by seq
 *
 *  Inputs: (variable names are important and begins with a colon)
 *      :fiId
 *
 *  Outputs: (order is important)
 *      fiId
 *      marketId
 *      symbolCaption
 *      marketCaption
 *      ownerCaption    // owner, issuer, provider naming is controversial
 */

-- GetSymbolsToFiId.sql
SELECT  s.fi_id,
        s.market_id,
        s.caption as symbol,
        m.caption as market,
        t.caption as provider

FROM :filu.symbol s
LEFT JOIN :filu.market AS m ON s.market_id = m.market_id
LEFT JOIN :filu.stype AS t ON s.stype_id = t.stype_id

WHERE fi_id = :fiId

ORDER BY t.seq
