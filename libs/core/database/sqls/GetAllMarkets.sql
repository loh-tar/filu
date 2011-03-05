/*
 *  Purpose:
 *     To fetch all markets with currency
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :
 *
 *  Outputs: (order is important)
 */

-- GetAllMarkets.sql
SELECT
  market.market_id AS "MarketId",
  market.caption   AS "Market",
  market.opentime  AS "OpenTime",
  market.closetime AS "CloseTime",
  fi.caption       AS "Currency",
  symbol.caption   AS "CurrencySymbol"

FROM
  :filu.market,
  :filu.fi,
  :filu.symbol

WHERE
  market.currency_fi_id = fi.fi_id and
  fi.fi_id = symbol.fi_id

ORDER BY
  market.caption ASC;
