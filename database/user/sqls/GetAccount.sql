/*
 *  Purpose:
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :accountId
 *     :depotId
 *     :fromDate (not yet)
 *     :toDate (not yet)
 *
 *  Outputs: (order *and* names are important)
 */

-- GetAccount.sql
SELECT  account_id AS "AccountId"
      , depot_id   AS "DebotId"
      , bdate      AS "Date"
      , btype      AS "Type"
      , btext      AS "Text"
      , bvalue     AS "Value"

FROM :user.account

WHERE CASE WHEN :accountId = -1 THEN true ELSE account_id = :accountId END
  and CASE WHEN :depotId   = -1 THEN true ELSE depot_id   = :depotId END

ORDER BY bdate;
