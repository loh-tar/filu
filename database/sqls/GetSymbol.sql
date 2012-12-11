/*
 *  Purpose:
 *     To fetch a given symbol
 *
 *  Inputs: (variable names are important and begins with a colon)
 *      :symbol
 *      :market
 *      :owner
 *
 *  Outputs: (order is important)
 *      SymbolId
 *      FiId
 *      MarketId
 *      Symbol
 *      Market
 *      Owner    // owner, issuer, provider naming is controversial
 */

-- GetSymbol.sql
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

WHERE s.caption = :symbol
  and m.caption = :market
  and t.caption = :owner
