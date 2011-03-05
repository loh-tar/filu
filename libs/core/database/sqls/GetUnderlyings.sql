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
  mfi.symbol       AS "Mother",
  ufi.symbol       AS "Comp",
  weight           AS "Weight",
  underlying_id    AS "UnderlyingId",
  ul.fi_id         AS "MotherId",
  underlying_fi_id AS "CompId"

FROM
  :filu.underlying ul
  JOIN :filu.lovelysymbol mfi USING(fi_id)
  JOIN :filu.lovelysymbol ufi ON(ufi.fi_id = underlying_fi_id)

WHERE CASE WHEN :motherId = 0 THEN TRUE ELSE ul.fi_id = :motherId END

ORDER BY
  mfi.symbol,
  ufi.symbol
