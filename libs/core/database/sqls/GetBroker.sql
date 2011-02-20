/*
 *  Purpose:
 *     Get one or all broker
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :brokerId
 */

-- GetBroker.sql
SELECT * FROM :filu.broker
WHERE CASE WHEN :brokerId = 0 THEN TRUE ELSE :brokerId = broker_id END
