/*
 *  Purpose:
 *     Fetch all existing underlyings
 *
 *  Inputs: (variable names are important and begins with a colon)
 *    :motherId
 *
 *  Outputs: (order is important)
 *    MotherSymbol
 *    Comp
 *    Weight
 *    UnderlyingId
 *    MotherId
 *    CompId
 */

-- GetUnderlyings.sql
SELECT
  underlying_id    AS "UnderlyingId",
  mfi.symbol       AS "Mother",
  ufi.symbol       AS "Comp",
  weight           AS "Weight",
  ul.fi_id         AS "MotherId",
  underlying_fi_id AS "CompId",
  fi.caption       AS "CompName",
  ft.caption       AS "CompType",
  ust.caption      AS "CompSymbolType",
  usm.caption      AS "CompSymbolMarket"

FROM
  :filu.underlying ul
  JOIN :filu.lovelysymbol mfi USING(fi_id)
  JOIN :filu.lovelysymbol ufi ON(ufi.fi_id = underlying_fi_id)
  JOIN :filu.fi fi ON(fi.fi_id = underlying_fi_id)
  JOIN :filu.ftype ft USING(ftype_id)
  JOIN :filu.symbol us ON(us.symbol_id = ufi.symbol_id)
  JOIN :filu.stype ust ON(ust.stype_id = us.stype_id)
  JOIN :filu.market usm ON(usm.market_id = us.market_id)

WHERE CASE WHEN :motherId = 0 THEN TRUE ELSE ul.fi_id = :motherId END

ORDER BY
  mfi.symbol,
  ufi.symbol
