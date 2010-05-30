*  file name: GetAllUnderlyings.sql
*  Used by function:
*
*  Purpose:
*     Fetch all existing underlyings
*
*  Inputs: (variable names are important and begins with a colon)
*
*
*  Outputs: (order is important)
*    MotherSymbol
*    UnderlyingSymbol
*    UnderlyingId
*    UnderlyingFiId
*    Weight
*    UnderlyingFiId

-- GetAllUnderlyings.sql
SELECT
  fi.symbol,
  ufi.symbol,
  underlying_id, ul.fi_id, weight, underlying_fi_id
FROM
  :filu.underlying ul
  join :filu.lovelysymbol fi using(fi_id)
  join :filu.lovelysymbol ufi on(ufi.fi_id=underlying_fi_id)

ORDER BY
  fi.symbol,
  ufi.symbol
;
