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

-- insert / update function
CREATE OR REPLACE FUNCTION <schema>.eodbar_insert
  ( nfi_id int8
  , nmarket_id int8
  , ndate date
  , nopen float
  , nhigh float
  , nlow float
  , nclose float
  , nvol float
  , noi float
  , nquality int2 )

  RETURNS void AS

$BODY$

DECLARE
  topen         float;
  thigh         float;
  tlow          float;
  tclose        float;
  tquality      int2;
  numrows       int4;

BEGIN
  -- set quality if null
  IF nquality IS NULL
  THEN tquality := 2; -- bronze, as tempo classified data
  ELSE tquality := nquality;
  END IF;

  -- check plausibility, especially yahoo tends to bugs in eod data
  topen  := nopen;
  thigh  := nhigh;
  tlow   := nlow;
  tclose := nclose;
  tquality := nquality;

  IF thigh < tclose
  THEN RAISE NOTICE 'fiId %, %: O:% H:% L:% C:%, close was > high', nfi_id, ndate, nopen, nhigh, nlow, nclose;
        thigh := tclose;
        tquality := 3;
        END IF;

  IF tclose < tlow
  THEN RAISE NOTICE 'fiId %, %: O:% H:% L:% C:%, close was < low', nfi_id, ndate, nopen, nhigh, nlow, nclose;
        tlow := tclose;
        tquality := 3;
        END IF;

  IF topen > thigh
  THEN RAISE NOTICE 'fiId %, %: O:% H:% L:% C:%, open was > high', nfi_id, ndate, nopen, nhigh, nlow, nclose;
        thigh := topen;
        tquality := 3;
        END IF;

  IF topen < tlow
  THEN RAISE NOTICE 'fiId %, %: O:% H:% L:% C:%, open was < low', nfi_id, ndate, nopen, nhigh, nlow, nclose;
        tlow := topen;
        tquality := 3;
        END IF;

  -- try to insert data
  BEGIN
    INSERT INTO <schema>.eodbar(fi_id, market_id, qdate, qopen, qhigh, qlow, qclose, qvol, qoi, quality)
            VALUES(nfi_id, nmarket_id, ndate, topen, thigh, tlow, tclose, nvol, noi, tquality);
    RETURN;
  EXCEPTION WHEN unique_violation
  THEN -- make an update
    if nvol = -1 -- in case of an update with bar data where no volume exist, like from onvista
    then
      update <schema>.eodbar
      set
        qopen          = topen,
        qhigh          = thigh,
        qlow           = tlow,
        qclose         = tclose,
        --qvol           = nvol,
        qoi            = noi,
        quality        = tquality
      where fi_id       = nfi_id
        and market_id   = nmarket_id
        and qdate       = ndate
        and quality     >= tquality;
    else
      update <schema>.eodbar
      set
        qopen          = topen,
        qhigh          = thigh,
        qlow           = tlow,
        qclose         = tclose,
        qvol           = nvol,
        qoi            = noi,
        quality        = tquality
      where fi_id       = nfi_id
        and market_id   = nmarket_id
        and qdate       = ndate
        and quality     >= tquality;
    end if;
  END;

  -- check the update result
  GET DIAGNOSTICS numrows = ROW_COUNT;

  IF numrows > 0 THEN
    IF (tquality <> 1) AND (tquality <> 2)
    THEN -- only info if something interessting
      RAISE NOTICE '<schema>.eodbar_insert() FI %, at date %. Updated to quality %', nfi_id, ndate, tquality;
    END IF;
  ELSE
    RAISE NOTICE '<schema>.eodbar_insert() Update for FI %, at date %. FAILS! new quality was %', nfi_id, ndate, tquality;
  END IF;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;

--
-- END OF FUNCTION <schema>.eodbar_insert
--
