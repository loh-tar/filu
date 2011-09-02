/*
 *  Purpose:
 *     Fetch all depots, optional filtered by owner or depotId
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :depotId
 *     :owner
 *
 *  Outputs: (order *and* names are important)
 *     depot_id   as "DepotId"
 *     caption    as "Name"
 *     trader     as "Trader"
 *     owner      as "Owner"
 *     currency   as "CurrencyId"
 *     currSymbol as "Currency"
 *     broker_id  as "BrokerId"
 *     br.caption as "Broker"
 */

-- GetDepots.sql.sql
SELECT  depot_id     AS "DepotId"
      , dp.caption   AS "Name"
      , trader       AS "Trader"
      , owner        AS "Owner"
      , currency     AS "CurrencyId"
      , ls.symbol    AS "Currency"
      , dp.broker_id AS "BrokerId"
      , br.caption   AS "Broker"
      , br.feeformula AS "FeeFormula"

  FROM :user.depot AS dp
  LEFT JOIN :filu.lovelysymbol AS ls ON fi_id=currency
  LEFT JOIN :filu.broker AS br ON dp.broker_id=br.broker_id

  WHERE CASE WHEN :depotId < 0 THEN true ELSE depot_id = :depotId END
    and CASE WHEN length(:owner) = 0  THEN true ELSE lower(owner) = lower(:owner) END
