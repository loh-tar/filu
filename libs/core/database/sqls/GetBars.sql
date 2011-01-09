*  file name: GetBars.sql
*  Used as query: sqlGetBars
*  Used in function: Filu::getBars(...)
*
*  Purpose:
*     Fetch bars of a user defined time frame
*
*  Inputs: (variable names are important and begins with a colon)
*     :fiId
*     :marketId
*     :fromDate
*     :toDate
*
*  Outputs: (order is important)
*     date // The most oldest quote must be the first in list
*     time
*     open
*     high
*     low
*     close
*     vol
*     oi

-- GetBars.sql
SELECT *
  FROM :filu.eodbar_get(:fiId, :marketId, :fromDate, :toDate, null, 1)

