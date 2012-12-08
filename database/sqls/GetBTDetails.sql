/*
 *  Purpose:
 *     Fetch details about a backtest strategy
 *
 *  Inputs: (variable names are important and begins with a colon)
 *
 *  Outputs: (order is important)
 */
-- GetBTDetails.sql
-- SELECT *
-- FROM :user.ts
-- WHERE ts_strategy = :strategy

-- SELECT ts.ts_id,
--        ts_strategy,
--        ts_fdate,
--        ts_tdate,
--        wltp ,
--        lltp,
--        agwlp,
--        alltp,
--        tpp,
--        score,
--        ts_rule,
--        ts_indicator
--
-- FROM :user.tsr, :user.ts
-- WHERE tsr.ts_id = ts.ts_id
-- and ts_strategy = :strategy

-- detail view

SELECT
  ts.ts_id,
  ts_strategy,
  ts_fdate,
  ts_tdate,
  symbol.caption,
  market.caption,
  wltp ,
  lltp,
  agwlp,
  alltp,
  tpp,
  score,
  ts_rule,
  ts_indicator

FROM
  :user.tsr
  LEFT JOIN :user.ts USING(ts_id)
  LEFT JOIN :filu.symbol ON tsr.fi_id = symbol.fi_id and tsr.market_id = symbol.market_id
  LEFT JOIN :filu.market ON tsr.market_id = market.market_id

WHERE
  ts_strategy = :strategy
