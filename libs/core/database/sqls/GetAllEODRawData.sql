/*
 *  Purpose:
 *     To fetch eod raw data. Sortet listed and optional filtered to
 *     given symbol type and/or market and/or FI type
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :fiId
 *     :fromDate
 *     :toDate
 *     :market     // like "Xetra"
 *
 *  Outputs: (order is important)
 */

-- GetAllEODRawData.sql
SELECT
  m.caption   AS "Market",
  e.qdate     AS "Date",
  e.qopen     AS "Open",
  e.qhigh     AS "High",
  e.qlow      AS "Low",
  e.qclose    AS "Close",
  e.qvol      AS "Volume",
  e.qoi       AS "OpenInterest",
  e.quality   AS "Quality"

FROM
  :filu.eodbar e,
  :filu.market m

WHERE
      e.fi_id     = :fiId
  and e.market_id = m.market_id
  and e.qdate BETWEEN :fromDate and :toDate
  and CASE WHEN length(:market) = 0  THEN true ELSE lower(m.caption) = lower(:market) END

ORDER BY
  m.caption,
  e.qdate ASC;
