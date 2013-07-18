/*
 *  Purpose:
 *     To fetch some Fi by long name and type settings
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :fiId
 *
 *  Outputs: (order is important)
 *     Id
 *     TypeId
 *     Name
 *     Type
 *     ExpiryDate
 */

-- GetFi.sql
SELECT f.fi_id, f.ftype_id, f.caption, t.caption, f.expirydate
  FROM :filu.fi f
  JOIN :filu.ftype t USING(ftype_id)
  WHERE  f.fi_id = :fiId
