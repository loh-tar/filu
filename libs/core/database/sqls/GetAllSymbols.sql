/*
 *  Purpose:
 *     To fetch FIs with all symbols. Redundant listed or optional filtered to
 *     given symbol type and/or market and/or FI type
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :ftype      // like "Stock"
 *     :market     // like "Xetra"
 *     :group      // like "MyFavorites"
 *
 *  Outputs: (order is important)
 */

-- GetAllSymbols.sql
-- All FIs symbols redundant listed with leading lovely symbol
-- and optional filtered to given symbol type and/or group membership
-- and/or market and/or FI type
SELECT
  ls.symbol       AS "RefSymbol",
  ft.caption      AS "Type",
  s.caption       AS "Symbol",
  st.caption      AS "Provider",
  m.caption       AS "Market",
  s.issuedate     AS "IDate",
  s.maturitydate  AS "MDate"

FROM
  :filu.fi f
  JOIN :filu.lovelysymbol ls USING(fi_id)
  JOIN :filu.ftype ft USING (ftype_id)
  JOIN :filu.symbol s USING (fi_id)
  JOIN :filu.market m USING (market_id)
  JOIN :filu.stype st USING (stype_id)

WHERE
  -- dont include lovely symbols here
  s.caption != (select symbol from :filu.lovelysymbol where fi_id = f.fi_id)
  and CASE WHEN length(:ftype)    = 0  THEN true ELSE ft.caption = :ftype END
  and CASE WHEN length(:market)   = 0  THEN true ELSE m.caption = :market END
  and CASE WHEN length(:group)    = 0  THEN true
           ELSE f.fi_id IN (select fi_id from :user.gmember gm
                                         where gm.group_id = :user.group_id_from_path(:group)) END

ORDER BY
  st.seq ASC,
  m.caption ASC,
  ft.caption ASC,
  f.caption ASC
;

