/*
 *  Purpose:
 *     Fetch the most newst bars of a Fi. The amount of bars
 *     is set by the user previous
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :fiId
 *     :marketId
 *     :limit the amount of quotes to fetch
 *     :toDate
 *
 *  Outputs: (order is important)
 *     date  // The most oldest quote must be the first in list
 *     time
 *     open
 *     high
 *     low
 *     close
 *     vol
 *     oi
 */

--GetBarsLtd.sql
SELECT *
  FROM :filu.eodbar_get(:fiId, :marketId, null, :toDate, :limit)
