*  file name: PutBacktest.sql
*  Used as query: mSqlPutBacktest
*  Used in function: FiluU::
*
*  Purpose:
*
*
*  Inputs: (variable names are important and begins with a colon)
*
*  Outputs: (order is important)


-- PutBacktest.sql
INSERT INTO :user.backtest (backtest_id, fdate, tdate, pgain, trule, tindicator, tname)
VALUES (:backtestId, :fromDate, :toDate, :gain, :rule, :indicator, :testName)

