*  file name: AddSymbol.sql
*  Used directly by Filu::addSymbol(...)
*       via Filu by Importer::addSymbol()
*
*  Purpose:
*     Add a new symbol to a FI
*
*  Inputs: (variable names are important and begins with a colon)
*     :fiId
*     :symbolId
*     :caption      // eg. AAPL
*     :sType        // eg. Yahoo
*     :market       // eg. Nyse
*
*  Outputs:
*    SymbolId OR error code
*

-- AddSymbol.sql
-- Insert, update or check the given Symbol.
-- Returns:
--   >0 the symbolId, if Symbol exist
--    0 if Symbol looks good but no fiId was given to add them
--   -1 SymbolType not valid
--   -2 Market not valid
--   -3 unique violation
--   -4 foreign key violation
SELECT * FROM :filu.symbol_insert(:fiId, :symbolId, :caption, :market, :sType);
