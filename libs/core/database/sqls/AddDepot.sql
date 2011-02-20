/*
 *  Purpose:
 *     Insert or update a depot.
 *     Returns Id or ErrorNo. All done by psql function
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :depotId
 *     :name
 *     :trader
 *     :owner
 *     :currency
 *     :broker
 *
 *  Outputs: (order is important)
 *     depotPosId or ErrorNo
 */

-- AddDepot.sql
SELECT * FROM :user.depot_insert(
    :depotId
  , :name
  , :trader, :owner, :currency, :broker
);
