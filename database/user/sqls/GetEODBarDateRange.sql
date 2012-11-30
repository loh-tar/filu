/*
 *  Purpose:
 *     Fetch the first and the last date of stored eodbars
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :fiId
 *     :marketId
 *     :quality
 *
 *  Outputs: (order is important)
 *     only one line with firstDate, lastDate
 *     firstDate is the oldes date
 *     lastDate the most recent date
 *     if no data in table, dont return any line
 */

-- GetEODBarDateRange.sql
SELECT *  FROM
  (SELECT MIN(qdate), MAX(qdate) FROM :filu.eodbar
    WHERE market_id = :marketId
      and fi_id = :fiId
      and quality < :quality) AS foo
WHERE max IS NOT NULL
