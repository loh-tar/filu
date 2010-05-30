*  file name: GetAllSymbolTypes.sql
*  Used by function Exporter::expSymbolTypes()
*                   Importer::reset()
*
*  Purpose:
*     To fetch all symbol types
*
*  Inputs: (variable names are important and begins with a colon)
*     :
*
*  Outputs: (order is important)
*

-- GetAllSymbolTypes.sql
SELECT
  stype.caption AS "SymbolType",
  stype.seq AS "SEQ",
  stype.isprovider AS "IsProvider"
FROM
  :filu.stype
ORDER BY
  stype.seq ASC,
  stype.caption ASC;
