/*
 *  Purpose:
 *     To fetch a list of symbols according ...some settings
 *  FIXME: needs a redisign or something else, its horror
 *
 *  Inputs: (variable names are important and begins with a colon)
 *      :symbol
 *      :market
 *      :onlyProviderSymbols
 *      :provider
 *      :fiId
 *      :ftype
 *
 *  Outputs: (order is important)
 *      fiId
 *      marketId
 *      symbolCaption
 *      marketCaption
 *      ownerCaption    // owner, issuer, provider naming is controversial
 */

-- GetSymbols.sql
SELECT s.fi_id, s.market_id, s.caption as symbol,
   CASE WHEN ms.caption IS NOT NULL
     THEN ms.caption ELSE m.caption  END as market,
   t.caption as provider,
   ft.caption as fi_type
FROM :filu.market m,:filu.stype t,
    :filu.fi f, :filu.ftype ft,
  (select min(st.seq) as seq, sy.fi_id
     from :filu.symbol sy,:filu.stype st
    where sy.stype_id = st.stype_id
      and sy.market_id > 0
      and st.isprovider >= :onlyProviderSymbols
    group by sy.fi_id
  ) as mysym,

  :filu.symbol s LEFT OUTER JOIN :filu.msymbol ms
  ON s.market_id = ms.market_id and s.stype_id = ms.stype_id

WHERE s.maturityDate >= current_date
AND s.market_id = m.market_id
AND s.stype_id = t.stype_id
AND t.isprovider >= :onlyProviderSymbols
--AND CASE WHEN TRUE = :onlyProviderSymbols THEN TRUE ELSE t.seq = mysym.seq END
AND f.fi_id = s.fi_id
AND f.ftype_id = ft.ftype_id
AND CASE WHEN :fiId = 0 THEN TRUE ELSE s.fi_id = mysym.fi_id END
AND CASE WHEN :fiId = 0 THEN TRUE ELSE f.fi_id = :fiId END
AND CASE WHEN length(:symbol) = 0 THEN TRUE ELSE s.caption = :symbol END
AND CASE WHEN length(:market) = 0 THEN TRUE ELSE m.caption = :market END
AND CASE WHEN length(:provider) = 0 THEN TRUE ELSE t.caption = :provider END
AND CASE WHEN length(:ftype) = 0 THEN TRUE ELSE ft.caption = :ftype END
GROUP BY s.caption, m.caption, s.fi_id, s.market_id, t.caption,
ft.caption, ms.caption
