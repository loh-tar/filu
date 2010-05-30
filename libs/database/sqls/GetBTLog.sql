*  file name: GetBTLog.sql
*  Used as query: mSqlGetBTLog
*  Used in function: FiluU::
*
*  Purpose:
*     Fetch
*
*  Inputs: (variable names are important and begins with a colon)
*     :backtestId
*     :fiId
*     :marketId
*
*  Outputs: (order is important)


-- GetBTLog.sql
SELECT btlog_id, backtest_id, fi_id, market_id, btdate, event, amount, money
  FROM :user.btlog
  WHERE backtest_id = (select MAX(backtest_id) from :user.btlog where fi_id = :fiId and market_id = :marketId)
  -- backtest_id = :backtestId

    AND fi_id = :fiId
    AND market_id = :marketId
  ORDER BY btlog_id
