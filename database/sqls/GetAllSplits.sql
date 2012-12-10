/*
 *  Purpose:
 *     To fetch all splits or optional filtered to
 *     given FI type and/or group membership
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :ftype      // like "Stock"
 *     :group      // like "MyFavorites"
 *
 *  Outputs: (order and names are important)
 *
 */

-- GetAllSplits.sql
SELECT
  ls.symbol   AS "Symbol",
  sp.sdate    AS "Date",
  sp.scomment AS "Comment",
  sp.sratio   AS "Ratio"

FROM
  :filu.split sp JOIN :filu.lovelysymbol ls USING(fi_id),
  :filu.fi f,
  :filu.ftype ft

WHERE
  sp.fi_id = f.fi_id
  and f.ftype_id = ft.ftype_id
  and sp.sdate BETWEEN :fromDate and :toDate
  and CASE WHEN length(:ftype) = 0  THEN true ELSE ft.caption = :ftype END
  and CASE WHEN length(:group) = 0
           THEN true
           ELSE f.fi_id IN (select fi_id from :user.gmember gm
                            where gm.group_id = :user.group_id_from_path(:group)) END

ORDER BY
  ls.symbol,
  sp.sdate ASC;
