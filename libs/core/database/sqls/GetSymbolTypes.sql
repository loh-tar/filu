*  file name: GetSymbolTypes.sql
*  Used in function: Filu::getSymbolTypes()
*
*  Purpose: Fetch symbol types
*
*  Inputs: (variable names are important and begins with a colon)
*     :all
*     :isProvider
*     :orderBySeq FIXME
*  Outputs: (order is important)
*     sTypeId
*     caption
*     seq
*     isProvider
*

-- GetSymbolTypes.sql
SELECT
  stype.stype_id AS "ID",
  stype.caption AS "SymbolType",
  stype.seq AS "SEQ",
  stype.isprovider AS "IsProvider"
FROM
  :filu.stype
WHERE
  CASE WHEN :all = TRUE THEN TRUE ELSE isprovider = :isProvider END
ORDER BY
  stype.seq ASC,
  stype.caption ASC;
