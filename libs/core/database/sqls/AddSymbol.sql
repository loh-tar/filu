/*
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
 */

-- AddSymbol.sql
SELECT * FROM :filu.symbol_insert(:fiId, :symbolId, :caption, :market, :sType);
