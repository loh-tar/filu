/*
 *  Purpose:
 *     To fetch FIs by a search string.
 *     The search string is fuzzy and used for names and symbols.
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :name          // like "pple"
 *     :type          // like "Stock"
 *     :hideNoMarket  // When true, don't select FIs where no provider symbols exist
 *
 *  Outputs: (order is important)
 *     FiId
 *     MarketId
 *     FiTypeId
 *     FiName
 *     FiType
 *     Symbol
 *     Market
 *     Provider
 */

-- SearchFi.sql
SELECT DISTINCT ON (f.fi_id)
      f.fi_id     as "FiId"
    , s.market_id as "MarketId"
    , t.ftype_id  as "TypeId"
    , f.caption   as "FiName"
    , t.caption   as "Type"
    , s.caption   as "Symbol"
    , m.caption   as "Market"
    , p.caption   as "Provider"

  FROM :filu.fi f, :filu.ftype t, :filu.symbol s, :filu.market m, :filu.stype p
  WHERE f.ftype_id = t.ftype_id
    and f.fi_id = s.fi_id
    and s.stype_id = p.stype_id
    and s.market_id = m.market_id
    and CASE WHEN :hideNoMarket  THEN s.market_id > 1 ELSE true END -- Don't list NoMarket
    and (lower(f.caption) LIKE '%'|| lower(:name) ||'%'
        or  EXISTS ( select s2.fi_id from :filu.symbol s2
                       where lower(s2.caption) LIKE '%'|| lower(:name) ||'%'
                         and s2.fi_id = s.fi_id
                   )
        )
    and lower(t.caption) LIKE '%'|| lower(:type)||'%'
