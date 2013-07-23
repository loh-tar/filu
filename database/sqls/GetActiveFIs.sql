/*
 *  Purpose:
 *     To fetch non locked FIs
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :marketId
 *
 *  Outputs: (order is important)
 *     FiId
 *     MarketId
 *     FiTypeId
 *     FiName
 *     FiType
 *     Symbol
 *     Market
 *     Provider
 */

-- GetActiveFIs.sql
SELECT
    f.fi_id     AS "FiId"
  , s.market_id AS "MarketId"
  , t.ftype_id  AS "TypeId"
  , f.caption   AS "FiName"
  , t.caption   AS "Type"
  , s.caption   AS "Symbol"
  , m.caption   AS "Market"
  , p.caption   AS "Provider"

FROM   :filu.fi f
  JOIN :filu.ftype t  USING (ftype_id)
  JOIN :filu.symbol s USING (fi_id)
  JOIN :filu.market m USING (market_id)
  JOIN :filu.stype p  USING (stype_id)

WHERE
  CASE WHEN :marketId > 1 THEN s.market_id = :marketId ELSE s.market_id > 1 END
  AND f.expirydate > (SELECT COALESCE(MAX(qdate), '1000-01-01')
                        FROM :filu.eodbar WHERE fi_id = f.fi_id)

ORDER BY f.caption
