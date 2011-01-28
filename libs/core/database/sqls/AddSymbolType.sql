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
SELECT * FROM :filu.stype_insert(:stypeId, :symbolType, :seq, :isProvider);
