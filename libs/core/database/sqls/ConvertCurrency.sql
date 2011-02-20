/*
 *  Purpose:
 *     Fetch all positions as sum view
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :money
 *     :sCurr
 *     :dCurr
 *     :date
 *
 *  Outputs: (order *and* names are important)
 *     converted money
 */

-- ConvertCurrency.sql
SELECT * FROM :filu.convert_currency(:money, :sCurr, :dCurr, :date)
