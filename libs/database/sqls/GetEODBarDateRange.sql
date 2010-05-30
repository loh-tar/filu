*  file name: GetEODBarDateRange.sql
*  Used as query: sqlGetEODBarDateRange
*  Used in function: Filu::getEODBarDateRange()
*
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

-- GetEODBarDateRange.sql
SELECT *  FROM
  (SELECT MIN(qdate), MAX(qdate) FROM :filu.eodbar
    WHERE market_id = :marketId
      AND fi_id = :fiId
      AND quality < :quality) AS foo
WHERE max IS NOT NULL
