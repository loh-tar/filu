*  file name: SearchFi.sql
*  Used as query:
*  Used in function: Filu
*
*  Purpose:
*     To
*
*  Inputs: (variable names are important and begins with a colon)
*     :name  // like "pple"
*     :type  // like "Stock"
*
*
*  Outputs: (order is important)
*     FiId
*     MarketId
*     FiName
*     FiType
*     Symbol
*     Market
*

-- SearchFi.sql
SELECT f.fi_id, s.market_id, f.caption, t.caption, s.caption, m.caption
  FROM :filu.fi f, :filu.ftype t,  :filu.symbol s,  :filu.market m
  WHERE f.ftype_id = t.ftype_id
    AND f.fi_id = s.fi_id
    AND s.market_id = m.market_id
    AND s.market_id != 0
    AND (UPPER(f.caption) LIKE '%'|| UPPER(:name) ||'%'
          OR UPPER(s.caption) LIKE '%'|| UPPER(:name) ||'%')
    AND UPPER(t.caption) LIKE '%'|| UPPER(:type)||'%'





