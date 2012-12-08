/*
 *  Purpose:
 *     Fetch
 *
 *  Inputs: (variable names are important and begins with a colon)
 *
 *  Outputs: (order is important)
 */

-- GetLastResults.sql
SELECT
  ts_strategy,
  ts.ts_id,
  avg(wltp)  as wltp,
  avg(lltp)  as lltp,
  avg(agwlp) as agwlp,
  avg(alltp) as alltp,
  avg(tpp)   as ttp,
  avg(score) as score

FROM :user.tsr
LEFT JOIN :user.ts USING(ts_id)
GROUP BY ts.ts_id, ts_strategy
ORDER BY ts_id DESC LIMIT 100
