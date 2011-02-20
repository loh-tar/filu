/*
 *  Purpose:
 *     Fetch
 *
 *  Inputs: (variable names are important and begins with a colon)
 *
 *  Outputs: (order is important)
 */

-- GetBacktest.sql
SELECT backtest_id, fdate, tdate, pgain, trule, tindicator, tname
  FROM :user.backtest
  --WHERE backtest_id = :backtestId
  ORDER BY pgain

SELECT * FROM :user.btlog
WHERE fi_id = 307
AND market_id = 302
AND backtest_id = (select MAX(backtest_id) from :user.btlog where fi_id = 307 and market_id = 302)
ORDER BY btlog_id
