/*
 *  Purpose:
 *     Fetch all positions as they are collected
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :depotId
 *     :fiId
 *
 *  Outputs: (order *and* names are important)
 */

-- GetDepotPositions.sql
SELECT depotpos_id AS "DepotPosId"
     , depot_id    AS "DepotId"
     , pdate       AS "Date"
     , d.fi_id     AS "FiId"
     , lsfi.symbol AS "Symbol"
     , fi.caption  AS "FI"
     , pieces      AS "Pieces"
     , price       AS "Price"
     , market_id   AS "MarketId"
     , lsc.symbol  AS "Currency"
     , m.caption   AS "Market"
     , note        AS "Note"

FROM :user.depotpos AS d
  JOIN :filu.fi USING(fi_id)
  JOIN :filu.lovelysymbol AS lsfi USING(fi_id)
  JOIN :filu.market AS m USING(market_id)
  JOIN :filu.lovelysymbol AS lsc ON lsc.fi_id = m.currency_fi_id

WHERE CASE WHEN :depotId = -1 THEN true ELSE depot_id = :depotId END
  and CASE WHEN :fiId    = -1 THEN true ELSE d.fi_id = :fiId END
;
