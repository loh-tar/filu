

     currently not used


*  file name: GetFiId.sql
*  Used as query: sqlGetFiId
*  Used in function: Filu::getFiId()
*
*  Inputs: (variable names are important and begins with a colon)
*     :symbol like "APPL" or "US0378331005"
*
*  Outputs: (order is important)
*     only the FiId
*

SELECT f.fi_id
  FROM :filu.fi f, :filu.symbol s
 WHERE f.fi_id=s.fi_id
   and s.caption=:symbol
