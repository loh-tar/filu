*  file name: GetFiIdBySymbol.sql
*  Used directly by Filu::setSymbolCaption(const QString &caption)
*       via Filu by Importer::addSymbol()
*
*
*  Purpose:
*     To fetch the FiId by any of its symbols
*
*  Inputs: (variable names are important and begins with a colon)
*     :symbol  // like "AAPL" or "US0378331005"
*
*  Outputs: (order is important)
*     FiId or error code
*

-- GetFiIdBySymbol.sql
-- Returns:
--   >0 if no problem, (the FiId)
--    0 if Symbol was not found
--   -1 if Symbol was found more than one time and was associated to different FIs
--   -2 if Symbol was empty
SELECT * FROM :filu.fiid_from_symbolcaption(:symbol)

