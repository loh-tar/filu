*--  file name: GetBestPerformance.sql
*--  Used with function: Filu::execSql()
*--
*--  Purpose:
*--     Fetch backtest results
*--
*--  Inputs: (variable names are important and begins with a colon)
*--
*--  Outputs: (order is important)

-- GetBestPerformance.sql
SELECT
  --ts.ts_id,
  ts_strategy,
  avg(wltp)  as wltp,
  avg(lltp)  as lltp,
  avg(agwlp) as agwlp,
  avg(alltp) as alltp,
  avg(tpp)   as tpp,
  avg(score) as score

FROM :user.tsr
LEFT JOIN :user.ts USING(ts_id)
GROUP BY
  --ts.ts_id,
 ts_strategy

ORDER BY
  tpp DESC LIMIT 100
