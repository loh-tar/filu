/*
 *  Purpose:
 *     Add or update splits
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :fiId
 *     :symbol
 *     :date
 *     :ratio
 *     :comment
 *     :quality
 */

--AddSplit.sql
SELECT :filu.split_insert(:symbol, :date, :ratio, :comment, cast(:quality as int2), :fiId);
