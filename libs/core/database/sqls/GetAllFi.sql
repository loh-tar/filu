*  file name: GetAllFi.sql
*  Used in function: FiluU::getFi()
*
*  Purpose:
*     To fetch FIs with all symbols. Redundant listed or optional filtered to
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

-- GetAllFi.sql
-- All FIs with full lovely symbol set
-- and optional filtered to given FI type
-- or group membership
SELECT

  f.caption       AS "Name",
  ft.caption      AS "Type",
  f.deletedate    AS "DDate",
  ls.symbol,
  --s.caption       AS "Symbol",
  st.caption      AS "Provider",
  m.caption       AS "Market",
  s.issuedate     AS "IDate",
  s.maturitydate  AS "MDate"

FROM
  --:filu.fi f
  --JOIN :filu.ftype ft USING (ftype_id)
  --JOIN :filu.symbol s USING (fi_id)
  --JOIN :filu.market m USING (market_id)
  --JOIN :filu.stype st USING (stype_id)
  :filu.fi f
  JOIN :filu.ftype ft USING (ftype_id)
  JOIN :filu.lovelysymbol ls USING(fi_id)
  JOIN :filu.symbol s USING (symbol_id)
  JOIN :filu.market m USING (market_id)
  JOIN :filu.stype st USING (stype_id)

WHERE

--  st.caption = (select distinct on (s2.fi_id) st.caption
--                  from :filu.symbol s2 join :filu.stype st using (stype_id)
--                  where f.fi_id = s2.fi_id
--                  order by s2.fi_id, st.seq asc)

  --AND
      CASE WHEN length(:ftype)    = 0  THEN true ELSE ft.caption = :ftype END
  and CASE WHEN length(:group)    = 0  THEN true
           ELSE f.fi_id IN (select fi_id from :user.gmember gm, :user.group g where gm.group_id = g.group_id and g.caption = :group) END

ORDER BY

  st.seq ASC,
  m.caption ASC,
  ft.caption ASC,
  f.caption ASC
;
