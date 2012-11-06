/*
 *  Purpose:
 *     To fix a range of bad bars. Yahoo delivers sometimes wrong
 *     adjusted bars after a split was done.
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :symbol
 *     :market
 *     :fromDate
 *     :toDate
 *     :ratio
 *     :quality
 */

--SplitBars.sql
UPDATE :filu.eodbar b
SET qopen  = qopen * :ratio
  , qhigh  = qhigh * :ratio
  , qlow   = qlow * :ratio
  , qclose = qclose * :ratio
  , quality = :quality
WHERE b.qdate BETWEEN :fromDate and :toDate
  and b.fi_id = :filu.fiid_from_symbolcaption(:symbol)
  and b.market_id = :filu.id_from_caption('market', :market)
