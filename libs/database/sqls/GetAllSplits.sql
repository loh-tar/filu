*  file name: GetAllSplits.sql
*  Used in function: ()
*
*  Purpose:
*     To fetch all splits or optional filtered to
*     given FI type and/or group membership
*
*  Inputs: (variable names are important and begins with a colon)
*     :ftype      // like "Stock"
*     :group      // like "MyFavorites"
*
*  Outputs: (order is important)
*     RefSymbol
*     sdate
*     scomment
*     sratio
*

-- GetAllSplits.sql
SELECT

  (select distinct on (s.fi_id) s.caption
    from :filu.symbol s, :filu.stype st
    where s.stype_id = st.stype_id and sp.fi_id = s.fi_id
    order by s.fi_id, st.seq asc) as RefSymbol,

  sp.sdate,
  sp.scomment,
  sp.sratio
FROM
  :filu.split sp,
  :filu.fi f,
  :filu.ftype ft
WHERE
  sp.fi_id = f.fi_id
  AND f.ftype_id = ft.ftype_id
  AND CASE WHEN length(:ftype) = 0  THEN true ELSE ft.caption = :ftype END
  AND CASE WHEN length(:group) = 0  THEN true
            ELSE f.fi_id IN (select fi_id from :user.gmember gm, :user.group g where gm.group_id = g.group_id AND g.caption = :group) END

ORDER BY
  RefSymbol,
  sp.sdate ASC;
