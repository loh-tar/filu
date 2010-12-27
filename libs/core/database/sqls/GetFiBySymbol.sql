*  file name: GetFiBySymbol.sql
*  Used directly by Filu::getFi(const QString &symbol)
*                   Filu::setSymbolCaption(const QString &caption)
*       via Filu by PerformerF.cpp
*                   Importer::addSymbol()
*
*
*  Purpose:
*     To fetch a Fi by any of its symbols
*
*  Inputs: (variable names are important and begins with a colon)
*     :symbol  // like "AAPL" or "US0378331005"
*
*  Outputs: (order is important)
*     Id
*     TypeId
*     Name
*     Type
*     IssueDate (controversial)
*     MaturityDate (controversial)
*

-- GetFiBySymbol.sql
SELECT DISTINCT f.fi_id, f.ftype_id, f.caption, t.caption
  FROM :filu.fi f, :filu.symbol s, :filu.ftype t
  WHERE f.fi_id = s.fi_id
    AND f.ftype_id = t.ftype_id
    AND s.caption = :symbol

