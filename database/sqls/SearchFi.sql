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
    and s.market_id > 1 -- Don't list NoMarket
    and (lower(f.caption) LIKE '%'|| lower(:name) ||'%'
        or  EXISTS ( select s2.fi_id from :filu.symbol s2
                       where lower(s2.caption) LIKE '%'|| lower(:name) ||'%'
                         and s2.fi_id = s.fi_id
                   )
        )
    and lower(t.caption) LIKE '%'|| lower(:type)||'%'
