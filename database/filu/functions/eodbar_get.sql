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

CREATE OR REPLACE FUNCTION :filu.eodbar_get
(
  aFiId        :filu.fi.fi_id%TYPE,
  aMarketId    :filu.market.market_id%TYPE,
  aFDate       :filu.eodbar.qdate%TYPE       DEFAULT '1000-01-01',
  aTDate       :filu.eodbar.qdate%TYPE       DEFAULT '3000-01-01',
  aLimit       int4                             DEFAULT 0  -- max number of rows
)
RETURNS SETOF :filu.fbar AS
$BODY$
DECLARE
  mHasFunc      int4;
  mFunc         varchar;

BEGIN

-- Something like that would be nice
-- http://www.mail-archive.com/pgsql-performance@postgresql.org/msg12945.html

  SELECT ftype.caption INTO mFunc
    FROM  :filu.ftype
      LEFT JOIN :filu.fi USING(ftype_id)
    WHERE fi.fi_id = aFiId;

  IF lower(mFunc) = 'stock' THEN -- Save some time
    --RAISE NOTICE 'As most times...Stock';
    RETURN QUERY SELECT * FROM :filu.gfi_stock_eodbar(aFiId, aMarketId, aFDate, aTDate, aLimit);
    RETURN;
  END IF;

  mFunc := 'gfi_' || lower(mFunc) || '_eodbar';
  mFunc := replace(mFunc, ' ', '_');
  mFunc := replace(mFunc, '.', '_');
  mFunc := replace(mFunc, '-', '_');

  SELECT INTO mHasFunc, mFunc
              proc.oid, proc.proname
    FROM  pg_namespace ns, pg_proc proc, pg_type rt
    WHERE ns.oid = proc.pronamespace
      and proc.prorettype = rt.oid
      and ns.nspname = ':filu'
      and proc.proname = mFunc;

  IF mHasFunc IS NULL THEN mFunc := 'gfi_rawdata_eodbar'; END IF;

  --RAISE NOTICE 'Used function is %', mFunc;
  RETURN QUERY EXECUTE 'SELECT * FROM :filu.' || quote_ident(mFunc) || ' ($1, $2, $3, $4, $5)'
                USING aFiId, aMarketId, aFDate, aTDate, aLimit;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION :filu.eodbar_get(...)
--
