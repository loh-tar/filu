/*
 *  Purpose:
 *     To fetch splits of a given FI
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :fiId
 *     :fromDate
 *     :toDate
 *
 *  Outputs: (order and names are important)
 *
 */

 -- GetSplits.sql
SELECT
  sdate    AS "Date",
  sratio   AS "Ratio",
  scomment AS "Comment",
  split_id,
  fi_id,
  quality

FROM :filu.split

WHERE
  fi_id = :fiId
  and sdate BETWEEN :fromDate and :toDate

ORDER BY
  sdate DESC;
