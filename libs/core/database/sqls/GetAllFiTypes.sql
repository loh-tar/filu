/*
 *  Purpose:
 *     To fetch all FI types
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :
 *
 *  Outputs: (order is important)
 *     FiType
 */

-- Get AllFiTypes.sql
SELECT
  ftype.caption AS "FiType"

FROM
  :filu.ftype

ORDER BY
  ftype.caption ASC;
