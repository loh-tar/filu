/*
 *  Purpose:
 *     To fetch FIs with all symbols. Redundant listed or optional filtered to
 *     given symbol type and/or market and/or FI type
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :ftype      // like "Stock"
 *     :group      // like "MyFavorites"
 *
 *  Outputs: (order *and* names are important)
 */

-- GetAllFi.sql
-- All FIs with full lovely symbol set
-- and optional filtered to given FI type
-- or group membership
SELECT
  f.caption       AS "Name",
  ft.caption      AS "Type",
  f.expirydate    AS "EDate",
  ls.symbol       AS "Symbol",
  st.caption      AS "Provider",
  m.caption       AS "Market",
  f.fi_id         AS "FiId"

FROM
  :filu.fi f
  JOIN :filu.ftype ft USING (ftype_id)
  JOIN :filu.lovelysymbol ls USING(fi_id)
  JOIN :filu.symbol s USING (symbol_id)
  JOIN :filu.market m USING (market_id)
  JOIN :filu.stype st USING (stype_id)

WHERE
      CASE WHEN length(:ftype)    = 0  THEN true ELSE ft.caption = :ftype END
  and CASE WHEN length(:group)    = 0  THEN true
           ELSE f.fi_id IN (select fi_id from :user.gmember gm
                                         where gm.group_id = :user.group_id_from_path(:group)) END

ORDER BY
  ft.caption ASC,
  f.caption ASC
;
