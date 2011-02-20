/*
 *  Purpose:
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :symbol like "APPL" or "US0378331005"
 *
 *  Outputs: (order is important)
 *     only the FiId
 */

     currently not used

SELECT f.fi_id
  FROM :filu.fi f, :filu.symbol s
 WHERE f.fi_id=s.fi_id
   and s.caption=:symbol
