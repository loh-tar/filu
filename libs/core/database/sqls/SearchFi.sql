/*
 *  Purpose:
 *     To
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :name  // like "pple"
 *     :type  // like "Stock"
 *
 *  Outputs: (order is important)
 *     FiId
 *     MarketId
 *     FiName
 *     FiType
 *     Symbol
 *     Market
 */

-- SearchFi.sql
SELECT f.fi_id, s.market_id, f.caption, t.caption, s.caption, m.caption
  FROM :filu.fi f, :filu.ftype t,  :filu.symbol s,  :filu.market m
  WHERE f.ftype_id = t.ftype_id
    and f.fi_id = s.fi_id
    and s.market_id = m.market_id
    and s.market_id != 0
    and (lower(f.caption) LIKE '%'|| lower(:name) ||'%'
          or lower(s.caption) LIKE '%'|| lower(:name) ||'%')
    and lower(t.caption) LIKE '%'|| lower(:type)||'%'


