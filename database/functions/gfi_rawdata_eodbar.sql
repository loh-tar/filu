/*
 *   This file is part of Filu.
 *
 *   Copyright (C) 2007, 2010, 2011  loh.tar@googlemail.com
 *
 *   Filu is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Filu is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Filu. If not, see <http://www.gnu.org/licenses/>.
 */

CREATE OR REPLACE FUNCTION <schema>.gfi_rawdata_eodbar
(
  aFiId        <schema>.fi.fi_id%TYPE,
  aMarketId    <schema>.market.market_id%TYPE,
  aFDate       <schema>.eodbar.qdate%TYPE       DEFAULT '1000-01-01',
  aTDate       <schema>.eodbar.qdate%TYPE       DEFAULT '3000-01-01',
  aLimit       int4                             DEFAULT 0 -- max number of rows
)
RETURNS SETOF <schema>.fbar AS
$BODY$
DECLARE
  mFDate        date;
  mTDate        date;

BEGIN

  mFDate := COALESCE(aFDate, '1000-01-01');
  mTDate := COALESCE(aTDate, '3000-01-01');

  -- fetch last date if limited rowcount needed
  IF aLimit > 0 THEN
      SELECT qdate INTO mFDate FROM <schema>.eodbar
       WHERE fi_id = aFiId
         and market_id = aMarketId
         and qdate <= mTDate
       ORDER BY qdate DESC
       LIMIT 1 offset aLimit-1;

      mFDate := COALESCE(mFDate, '1000-01-01');
  END IF;

  RETURN QUERY SELECT e.qdate, '23:59:59'::time
                    , cast(e.qopen  as float)
                    , cast(e.qhigh  as float)
                    , cast(e.qlow   as float)
                    , cast(e.qclose as float)
                    , cast(e.qvol   as float)
                    , cast(e.qoi    as float)
                   FROM <schema>.eodbar e
                   WHERE e.fi_id = aFiId
                     and qdate BETWEEN mFDate and mTDate
                     and market_id = aMarketId
                   ORDER BY qdate ASC;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;

--
-- END OF FUNCTION <schema>.gfi_rawdata_eodbar(...)
--
