/*
 *  Purpose:
 *     To fetch the FiId by any of its symbols
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :symbol  // like "AAPL" or "US0378331005"
 *
 *  Outputs: (order is important)
 *     FiId or error code
 */

-- GetFiIdBySymbol.sql
SELECT * FROM :filu.fiid_from_symbolcaption(:symbol)
