/*
 *  Purpose:
 *     Add a new FI to the DB
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :fiId
 *     :name
 *     :fType
 *     :symbol
 *     :sType
 *     :market
 */

-- AddFi.sql Returns:
SELECT * FROM :filu.fi_insert(:fiId, :name, :fType, :symbol, :sType, :market);
