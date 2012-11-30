/*
 *  Purpose:
 *     To delete a range of eodbars.
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :symbol
 *     :market
 *     :fromDate
 *     :toDate
 */

--DeleteBars.sql
DELETE FROM :filu.eodbar b
WHERE b.qdate BETWEEN :fromDate and :toDate
  and b.fi_id = :filu.fiid_from_symbolcaption(:symbol)
  and b.market_id = :filu.id_from_caption('market', :market)
