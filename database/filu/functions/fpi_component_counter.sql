/*
 *   This file is part of Filu.
 *
 *   Copyright (C) 2007, 2010, 2011, 2012  loh.tar@googlemail.com
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
/*
    FiMi

    Copyright (C) 2001-2006 Christian Kindler

    This file is part of FiMi.

    FiMi is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

CREATE OR REPLACE FUNCTION :schema.fpi_ccp
  (
    fiId      bigint,
    marketId  bigint,
    fdate     date,
    tdate     date
  )
RETURNS SETOF :schema.fdouble AS
$BODY$

DECLARE
  mRecord    record;
  mResult    :schema.fdouble;
  mULcount   int; -- underlying count
--
--LongName:Component Counter Percentage
--Group:Counter
--Author:Christian Kindler
--Desc:Look at all underlyings from a FI and add up if they have a winday(+1)
--Desc:or a lostday(-1) and returns the result as percentage value.
--Input1:FiRef
--Output1:ccp, HistogramBar, The Beef
--Error1:FI has no underlyings
--
BEGIN
  -- Check how many underlying has the FI
  PERFORM underlying_fi_id
    FROM :schema.underlying
    WHERE fi_id = fiId;

  GET DIAGNOSTICS mULcount = ROW_COUNT;

  IF mULcount = 0
  THEN
--     -- No underlyings, create and return a dummy result
--     mResult.fdate := fdate;
--     FOR mRecord IN
--     SELECT qdate
--       FROM :schema.eodbar
--       WHERE market_id = marketId
--         and qdate BETWEEN fdate and tdate
--         and fi_id = fiId
--       ORDER BY qdate ASC
--
--     LOOP
--         mResult.fdate := mRecord.qdate;
--         mResult.ftime := '23:59:59'::Time;
--         mResult.fdata := 0.0;
--
--         RETURN NEXT mResult;
--     END LOOP;
--     RETURN;

      -- No underlyings, return an error code
      mResult.fdate := '1000-01-01'::Date; -- Mark data as error data
      mResult.ftime := '23:59:59'::Time;   -- Obsolete but anyway
      mResult.fdata := 1;                  -- Error No
    RETURN NEXT mResult;
    RETURN;
  END IF;

  FOR mRecord IN
  SELECT e1.qdate,
        (SUM(CASE WHEN e2.qopen > e2.qclose THEN -1
                  WHEN e2.qopen < e2.qclose THEN  1
                  ELSE 0 END))::float8 AS beef

    FROM :schema.eodbar e1, :schema.eodbar e2
  WHERE 1=1
    and e1.market_id = marketId
    and e1.qdate BETWEEN fdate and tdate
    and e1.qdate = e2.qdate
    and e1.market_id = e2.market_id
    and e2.fi_id in (SELECT underlying_fi_id
                        FROM :schema.underlying
                      WHERE fi_id = fiId)
    and e1.fi_id = fiId
  GROUP BY e1.qdate
  ORDER BY e1.qdate ASC

  LOOP
      mResult.fdate := mRecord.qdate;
      mResult.ftime := '23:59:59'::Time;
      mResult.fdata := (mRecord.beef / mULcount) * 100;

      RETURN NEXT mResult;
  END LOOP;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION :schema.fpi_ccp(...)
--
