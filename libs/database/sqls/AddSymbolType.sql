*  file name: AddSymbolType
*  Used in function: Filu::addSymbolType
*
*  Purpose:
*     Add or update symbol types
*
*  Inputs: (variable names are important and begins with a colon)
*     :stypeId
*     :symbolType
*     :seq
*     :isProvider

-- AddSymbolType.sql
-- Returns:
--   >0 if all is fine, the SymbolTypeId
--   -1 if SymbolType was empty
SELECT * FROM :filu.stype_insert(:stypeId, :symbolType, :seq, :isProvider);
