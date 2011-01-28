/*
 *   This file is part of Filu.
 *
 *   Copyright (C) 2007, 2010  loh.tar@googlemail.com
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

CREATE OR REPLACE FUNCTION <schema>.eodbar_get
(
  aFiId        <schema>.fi.fi_id%TYPE,
  aMarketId    <schema>.market.market_id%TYPE,
  aFDate       <schema>.eodbar.qdate%TYPE       DEFAULT '1000-01-01',
  aTDate       <schema>.eodbar.qdate%TYPE       DEFAULT '3000-01-01',
  aLimit       int4                             DEFAULT 0  -- max number of rows
)
RETURNS SETOF <schema>.fbar AS
$BODY$
DECLARE
  mHasFunc      int4;
  mFunc         varchar;

BEGIN

-- Something like that would be nice
-- http://www.mail-archive.com/pgsql-performance@postgresql.org/msg12945.html

  SELECT INTO mFunc ftype.caption
    FROM  <schema>.ftype
      LEFT JOIN <schema>.fi USING(ftype_id)
    WHERE fi.fi_id = aFiId;

  mFunc := 'gfi_' || lower(mFunc) || '_eodbar';
  mFunc := replace(mFunc, ' ', '_');
  mFunc := replace(mFunc, '.', '_');
  mFunc := replace(mFunc, '-', '_');

  SELECT INTO mHasFunc, mFunc
              proc.oid, proc.proname
    FROM  pg_namespace ns, pg_proc proc, pg_type rt
    WHERE ns.oid = proc.pronamespace
      and proc.prorettype = rt.oid
      and ns.nspname = '<schema>'
      and proc.proname = mFunc;

  IF mHasFunc IS NULL THEN mFunc := 'gfi_rawdata_eodbar'; END IF;

  IF mFunc = 'gfi_stock_eodbar' -- Does it make sense to check this to save some time?
  THEN
    --RAISE NOTICE 'As most times...Stock';
    RETURN QUERY SELECT * FROM <schema>.gfi_stock_eodbar(aFiId, aMarketId, aFDate, aTDate, aLimit);

  ELSE
    --RAISE NOTICE 'Used function is %', mFunc;
    RETURN QUERY EXECUTE 'SELECT * FROM <schema>.' || quote_ident(mFunc) || ' ($1, $2, $3, $4, $5)'
                 USING aFiId, aMarketId, aFDate, aTDate, aLimit;
  END IF;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION <schema>.eodbar_get(...)
--
