*  file name: GetAllCOs.sql
*  Used by function:  Exporter::expCOs()
*
*
*  Purpose:
*     Fetch all Chart Objects of a user.
*     Why another sql besides of GetCOs.sql? Here we have symbols
*     as input and output parameter
*
*  Inputs: (variable names are important and begins with a colon)
*     :ftype      // like "Stock"
*     :provider   // like "ISIN"
*     :market     // like "Xetra"
*     :group      // like "MyFavorites"
*
*  Outputs: (order is important)
*     RefSymbol
*     Market
*     CODate
*     Plot
*     Type
*     ArgV

-- GetAllCOs.sql
SELECT
  ( select distinct on (s2.fi_id) s2.caption
      from :filu.symbol s2
      join :filu.stype st using (stype_id)
      where f.fi_id = s2.fi_id
      order by s2.fi_id, st.seq asc

  )               AS "RefSymbol",

  m.caption       AS "Market",
  co.co_date      AS "CODate",
  co.co_plot      AS "Plot",
  co.co_type      AS "Type",
  co.co_argv      AS "ArgV"

FROM
  :user.co co
  JOIN :filu.market m USING (market_id)
  JOIN :filu.fi f     USING (fi_id)
  JOIN :filu.ftype ft USING (ftype_id)

WHERE
  --AND (co_date BETWEEN :fromDate AND :toDate)
      CASE WHEN length(:ftype)    = 0  THEN true ELSE ft.caption = :ftype END
  AND CASE WHEN length(:group)    = 0  THEN true
           ELSE f.fi_id IN (select fi_id from :user.gmember gm, :user.group g where gm.group_id = g.group_id AND g.caption = :group) END

ORDER BY

  co_date,
  m.caption ASC,
  ft.caption ASC
;