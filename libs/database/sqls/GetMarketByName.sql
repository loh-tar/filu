*  file name: GetMarketByName.sql
*  Used in function: Filu::getMarket()
*
*  Purpose:
*
*  Inputs: (variable names are important and begins with a colon)
*     :market  // like "NYSE" or "XETRA" etc
*
*  Outputs: (order is important)
*     market id
*     market name
*     market currency
*     market type id
*

-- GetMarketByNameeee.sql
SELECT market_id, caption, currency_fi_id--, mtype_id
  FROM :filu.market
  WHERE CASE WHEN length(:market) = 0 THEN TRUE ELSE caption = :market END
