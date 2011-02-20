/*
 *  Purpose:
 *
 *  Inputs: (variable names are important and begins with a colon)
 *
 *  Outputs: (order is important)
 */

-- PutBTLog.sql
INSERT INTO :user.btlog (backtest_id, fi_id, market_id, btdate, event, amount, money)
VALUES (:backtestId, :fiId, :marketId, :date, :event, :amount, :money)
