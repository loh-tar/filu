/*
 *  Purpose:
 *     Fetch all Chart Objects of a user.
 *     Why another sql besides of GetCOs.sql? Here we have symbols
 *     as input and output parameter
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :fromDate
 *     :toDate
 *     :ftype      // like "Stock"
 *     :group      // like "MyFavorites"
 *
 *  Outputs: (order is important)
 *     RefSymbol
 *     Market
 *     CODate
 *     Plot
 *     Type
 *     ArgV
 */

-- GetAllCOs.sql
SELECT
  ls.symbol       AS "RefSymbol",
  m.caption       AS "Market",
  co.co_date      AS "CODate",
  co.co_plot      AS "Plot",
  co.co_type      AS "Type",
  co.co_argv      AS "ArgV"

FROM
  :user.co co
  JOIN :filu.lovelysymbol ls USING(fi_id)
  JOIN :filu.market m USING (market_id)
  JOIN :filu.fi f     USING (fi_id)
  JOIN :filu.ftype ft USING (ftype_id)

WHERE
      co_date BETWEEN :fromDate and :toDate
  and CASE WHEN length(:ftype)    = 0  THEN true ELSE ft.caption = :ftype END
  and CASE WHEN length(:group)    = 0  THEN true
           ELSE f.fi_id IN (select fi_id from :user.gmember gm
                                         where gm.group_id = :user.group_id_from_path(:group)) END

ORDER BY
  co_date,
  m.caption ASC,
  ft.caption ASC
;