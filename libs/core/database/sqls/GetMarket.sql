/*
 *  Purpose:
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :market     // like "NYSE" or "XETRA" etc
 *     :marketId
 *
 *  Outputs: (order is important)
 *     market id
 *     market name
 *     currency id
 *     currency name
 *     currency symbol
 *
 * see also GetAllMarkets.sql
 */

-- GetMarket.sql
SELECT m.market_id, m.caption, currency_fi_id, fi.caption, s.symbol
  FROM :filu.market m
  JOIN :filu.fi ON currency_fi_id = fi_id
  JOIN :filu.lovelysymbol s ON s.fi_id = fi.fi_id
  WHERE CASE WHEN length(:market) = 0 THEN TRUE ELSE lower(m.caption) = lower(:market) END
    and CASE WHEN :marketId = 0 THEN TRUE ELSE m.market_id = :marketId END
