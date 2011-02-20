/*
 *  Purpose:
 *     Insert or update an account postion.
 *     Returns Id or ErrorNo. All done by psql function
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :depotId
 *     :date
 *     :type
 *     :text
 *     :value
 *     :accountId
 *
 *  Outputs: (order is important)
 *     accountId or ErrorNo
 */

-- AddAccountPos.sql
SELECT * FROM :user.account_insert(
    :accountId, :depotId, :date
  , CAST(:type as smallint), :text, :value
);
