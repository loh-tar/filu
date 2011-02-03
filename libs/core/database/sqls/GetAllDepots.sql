/*  file name: GetAllDepots.sql
 *  Used in function: FiluU::
 *
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
 */

-- GetAllDepots.sql.sql
SELECT  depot_id  as "DepotId"
      , caption   as "Name"
      , trader    as "Trader"
      , owner     as "Owner"
      , currency  as "CurrencyId"
      , ls.symbol as "Currency"
      , broker_id as "BrokerId"
  FROM :user.depot
  LEFT JOIN :filu.lovelysymbol AS ls ON fi_id=currency
