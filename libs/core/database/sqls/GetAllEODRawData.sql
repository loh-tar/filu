*  file name: GetAllEODRawData.sql
*  Used in function:
*
*  Purpose:
*     To fetch eod raw data. Sortet listed and optional filtered to
*     given symbol type and/or market and/or FI type
*
*  Inputs: (variable names are important and begins with a colon)
*     :ftype      // like "Stock"
*     :provider   // like "ISIN"
*     :market     // like "Xetra"
*     :group      // like "MyFavorites"
*
*  Outputs: (order is important)
*

-- GetAllEODRawData.sql
SELECT

(select distinct on (s.fi_id) s.caption
  from :filu.symbol s, :filu.stype st
  where s.stype_id = st.stype_id and f.fi_id = s.fi_id
  order by s.fi_id, st.seq asc)  as "RefSymbol",

  m.caption,
  e.qdate,
  e.qopen,
  e.qhigh,
  e.qlow,
  e.qclose,
  e.qvol,
  e.qoi,
  e.quality
FROM
  :filu.eodbar e,
  :filu.fi f,
  :filu.ftype ft,
  :filu.market m,
  :filu.stype st,
  :filu.symbol s
WHERE
      e.fi_id = s.fi_id
  and f.ftype_id = ft.ftype_id
  and f.fi_id = s.fi_id
  and s.market_id = m.market_id
  and st.stype_id = s.stype_id
  and e.market_id = m.market_id
  and CASE WHEN length(:ftype) = 0  THEN true ELSE ft.caption = :ftype END
  and CASE WHEN length(:provider) = 0  THEN true ELSE st.caption = :provider END
  and CASE WHEN length(:market) = 0  THEN true ELSE m.caption = :market END
  and CASE WHEN length(:group) = 0  THEN true
           ELSE f.fi_id IN (select fi_id from :user.gmember gm, :user.group g where gm.group_id = g.group_id and g.caption = :group) END
ORDER BY
  m.caption ASC,
  s.caption ASC,
  ft.caption ASC,
  e.qdate ASC;
