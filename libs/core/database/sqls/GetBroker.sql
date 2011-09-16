/*
 *  Purpose:
 *     Get one or all broker
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :brokerId
 */

-- GetBroker.sql
SELECT broker_id      AS "BrokerId"
     , caption        AS "Name"
     , currency_fi_id AS "CurrencyId"
     , ls.symbol      AS "Currency"
     , feeformula     AS "FeeFormula"
     , quality        AS "Quality"

FROM :filu.broker
JOIN :filu.lovelysymbol AS ls ON currency_fi_id = fi_id

WHERE CASE WHEN :brokerId = 0 THEN TRUE ELSE :brokerId = broker_id END;
