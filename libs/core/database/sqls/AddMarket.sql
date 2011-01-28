*  file name: AddMarket.sql
*  Used in function: Filu::addMarket
*
*  Purpose:
*     Add a new market to the DB
*
*  Inputs: (variable names are important and begins with a colon)
*     :market
*     :currencyName
*     :currencySymbol
*

-- AddMarket.sql
SELECT * FROM :filu.market_insert(:market, :currencyName, :currencySymbol);


