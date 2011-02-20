/*
 *  Purpose:
 *     Fetch all depots
 *
 *  Inputs: (variable names are important and begins with a colon)
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

-- GetAllDepots.sql.sql
SELECT  depot_id     AS "DepotId"
      , dp.caption   AS "Name"
      , trader       AS "Trader"
      , owner        AS "Owner"
      , currency     AS "CurrencyId"
      , ls.symbol    AS "Currency"
      , dp.broker_id AS "BrokerId"
      , br.caption   AS "Broker"

  FROM :user.depot AS dp
  LEFT JOIN :filu.lovelysymbol AS ls ON fi_id=currency
  LEFT JOIN :filu.broker AS br ON dp.broker_id=br.broker_id
