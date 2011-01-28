*  file name: GetGMembers.sql
*  Used as query: sqlGetGMembers
*  Used in function: FiluU::
*
*  Purpose:
*     Fetch
*
*  Inputs: (variable names are important and begins with a colon)
*     :groupId
*
*  Outputs: (order is important)
*     gmember_id
*     fi_id
*     symbol
*     market
*     market_id

-- GetGMembers.sql
SELECT
  g.gmember_id,
  g.fi_id,
  ls.symbol,
  m.caption AS market,
  m.market_id

FROM
  :user.gmember g
  JOIN :filu.lovelysymbol ls using (fi_id)
  JOIN :filu.symbol s USING (fi_id)
  JOIN :filu.market m USING (market_id)

WHERE
  g.group_id = :groupId
  and m.caption != 'NoMarket'

ORDER BY s.caption;
