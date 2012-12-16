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
 */

-- GetMarket.sql
SELECT
  m.market_id    AS "MarketId",
  m.caption      AS "MarketSymbol",
  m.opentime     AS "OpenTime",
  m.closetime    AS "CloseTime",
  currency_fi_id AS "CurrencyId",
  fi.caption     AS "Currency",
  s.symbol       AS "CurrencySymbol"

FROM
       :filu.market m
  JOIN :filu.fi ON currency_fi_id = fi_id
  JOIN :filu.lovelysymbol s ON s.fi_id = fi.fi_id

WHERE
      CASE WHEN length(:market) = 0 THEN TRUE ELSE lower(m.caption) = lower(:market) END
  and CASE WHEN :marketId < 1  THEN TRUE ELSE m.market_id = :marketId END
  and CASE WHEN :marketId = -1 THEN m.market_id > 2  ELSE TRUE END -- Ignore NoMarket and Forex

ORDER BY
  m.caption ASC;
