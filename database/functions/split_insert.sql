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

CREATE OR REPLACE FUNCTION <schema>.split_insert
  (
    nsymbol    <schema>.symbol.caption%TYPE,
    ndate      <schema>.split.sdate%TYPE,
    nratio     <schema>.split.sratio%Type,
    ncomment   <schema>.split.scomment%TYPE,
    nquality   int, --<schema>.split.quality%TYPE, does not work,
                    -- function will not recognised when called, need than ::smallint as typecast
    nfiId      <schema>.fi.fi_id%TYPE DEFAULT 0
  )
  RETURNS int2 AS
$BODY$

DECLARE
  fiId       <schema>.fi.fi_id%TYPE;
  myRecord record;

BEGIN
  -- Update or inster split data.
  -- Returns:
  --   >0 if all was fine, the new split_id
  --   -1 if symbol not found
  --   -2 if symbol exist more than one times to different FIs
  --   -3 if New Quality is not good enough

  fiId := nfiId;

  IF fiId < 1
  THEN
    -- -2= empty caption, -1=more than one found and was associated to different FIs, 0=unknown, >1=id
    fiId := <schema>.fiId_from_symbolcaption(nsymbol);
    IF (fiId =  0) OR (fiId = -2 ) THEN RETURN -1; END IF;
    IF (fiId = -1) THEN RETURN -2; END IF;
  END IF;

  -- check if data exist
  SELECT INTO myRecord split_id, quality
  FROM <schema>.split
  WHERE
    fi_id   = fiId AND
    sdate   = ndate;

  IF myRecord.split_id IS NULL
  THEN -- make the insert

    myRecord.split_id := nextval('<schema>.split_split_id_seq');
    INSERT  INTO <schema>.split(split_id, fi_id, sdate, sratio, scomment, quality)
            VALUES(myRecord.split_id, fiId, ndate, nratio, ncomment, nquality);

    RETURN myRecord.split_id;
  END IF;

  IF myRecord.quality < nquality THEN RETURN -3; END IF;

  -- make an update
  UPDATE <schema>.split
    SET sdate     = ndate,
        sratio    = nratio,
        scomment  = ncomment,
        quality   = nquality
    WHERE split_id  = myRecord.split_id;

  RETURN myRecord.splitId;

END
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION <schema>.split_insert
--
