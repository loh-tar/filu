*  file name: AddUnderlying.sql
*  Used in function: Filu::addUnderlying
*
*  Purpose:
*     Add a underlyings
*
*  Inputs: (variable names are important and begins with a colon)
*     :motherSymbol
*     :symbol
*     :weight
*

--AddUnderlying.sql
SELECT * FROM :filu.underlying_insert(:motherSymbol, :symbol, :weight);
