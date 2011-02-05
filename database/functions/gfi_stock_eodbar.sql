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

CREATE OR REPLACE FUNCTION <schema>.gfi_stock_eodbar
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
  mQuery       varchar;
  mRecord      record;
  mFDate       date;
  mResult      <schema>.fbar;

BEGIN

  -- fetch last date if limited rowcount needed
  IF aLimit > 0 THEN
      SELECT INTO mFDate qdate FROM <schema>.eodbar
       WHERE fi_id = aFiId
         and market_id = aMarketId
         and qdate <= aTDate
       ORDER BY qdate DESC
       LIMIT aLimit offset aLimit-1;
  END IF;

  mFDate := COALESCE(mFDate, aFDate);

  mQuery := 'SELECT *,
                COALESCE((select exp(sum(ln(sratio)))
                            from <schema>.split s
                            where s.sdate > e.qdate and s.fi_id = e.fi_id), 1)
                            as sratio

              FROM <schema>.eodbar e
              WHERE
                e.fi_id = $1
                and qdate BETWEEN $2 and $3
                and market_id = $4
              ORDER BY qdate ASC';

  -- adjust data to splits
  FOR mRecord IN EXECUTE mQuery USING aFiId, mFDate, aTDate, aMarketId
  LOOP
    mResult.fdate  := mRecord.qdate;
    mResult.ftime  := '23:59:59'::Time;

    mResult.fopen  := mRecord.qopen  * mRecord.sratio;
    mResult.fhigh  := mRecord.qhigh  * mRecord.sratio;
    mResult.flow   := mRecord.qlow   * mRecord.sratio;
    mResult.fclose := mRecord.qclose * mRecord.sratio;
    mResult.fvol   := mRecord.qvol ;--  / mRecord.sratio;

    mResult.foi    := mRecord.qoi;

    RETURN NEXT mResult;
  END LOOP;

  RETURN;
END
$BODY$
LANGUAGE PLPGSQL VOLATILE;

--
-- END OF FUNCTION <schema>.gfi_stock_eodbar(...)
--
