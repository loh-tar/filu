/*
 *  Purpose:
 *     To fetch all symbols a FI has ordered by seq
 *
 *  Inputs: (variable names are important and begins with a colon)
 *      :fiId
 *
 *  Outputs: (order is important)
 *      SymbolId
 *      FiId
 *      MarketId
 *      Symbol
 *      Market
 *      Owner    // owner, issuer, provider naming is controversial
 */

-- GetSymbolsToFiId.sql
SELECT
    s.symbol_id as "SymbolId",
    s.fi_id     as "FiId",
    s.market_id as "MarketId",
    s.caption   as "Symbol",
    m.caption   as "Market",
    t.caption   as "Owner",
    s.quality   as "Quality"

FROM :filu.symbol s
JOIN :filu.market as m USING(market_id)
JOIN :filu.stype  as t USING(stype_id)

WHERE fi_id = :fiId

ORDER BY t.seq
