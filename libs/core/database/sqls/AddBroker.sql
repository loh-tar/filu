/*
 *  Purpose:
 *     Add a new broker to the DB
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :name
 *     :feeFormula
 *     :quality
 *     :brokerId
 */

-- AddBroker.sql
SELECT * FROM :filu.broker_insert(:name, :feeFormula, cast(:quality as int2), cast(:brokerId as int4));
