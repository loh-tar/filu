/*
 *  Purpose:
 *     
 *
 *  Inputs: (variable names are important and begins with a colon)
 */

-- AddTradingStrategy.sql
INSERT INTO  :user.ts (ts_id, ts_strategy, ts_fdate, ts_tdate, ts_rule, ts_indicator)
VALUES (:tsId, :sId, :fromDate, :toDate, :rule, :indicator)
