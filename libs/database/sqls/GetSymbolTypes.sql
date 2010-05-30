* FIXME: unsure if needed, currently not used. see also GetAllSymbolTypes.sql
*  file name: GetSymbolTypes.sql
*  Used as query:
*  Used in function: Filu::
*
*  Purpose: Fetch symbol types
*
*  Inputs: (variable names are important and begins with a colon)
*     :all
*     :onlyProvider
*  Outputs: (order is important)
*     sTypeId
*     caption
*     seq
*     isProvider
*

-- GetSymbolTypes.sql
SELECT stype_id, caption, seq, isprovider
FROM :filu.stype
WHERE
CASE WHEN :all = TRUE THEN TRUE ELSE isprovider = :onlyProvider END
;