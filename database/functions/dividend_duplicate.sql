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

CREATE OR REPLACE FUNCTION <schema>.dividend_duplicate()
RETURNS TRIGGER AS
$BODY$

DECLARE
  mExist    <schema>.dividend.dividend_id%TYPE;

BEGIN

  -- try to update data
  UPDATE <schema>.dividend
      SET ddate     = new.ddate,
          dpayout   = new.dpayout,
          dcomment  = new.dcomment
      WHERE fi_id   = new.fi_id and
            ddate   = new.ddate and
            quality >= new.quality;

  IF FOUND THEN RETURN NULL; END IF;

  -- check if data exist but quality to bad
  SELECT INTO mExist dividend_id
      FROM <schema>.dividend
      WHERE
        fi_id   = new.fi_id and
        ddate   = new.ddate;

  IF mExist IS NOT NULL THEN RETURN NULL; END IF;

  -- no, its not here, make an insert
  IF COALESCE(new.dividend_id, 0) = 0 -- but only if id is not set
  THEN new.dividend_id := nextval('<schema>.dividend_dividend_id_seq'); END IF;

  RETURN new;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;

DROP TRIGGER IF EXISTS <schema>_dividend_duplicates ON <schema>.dividend;

CREATE TRIGGER <schema>_dividend_duplicates
  BEFORE INSERT
  ON <schema>.dividend
  FOR EACH ROW
  EXECUTE PROCEDURE <schema>.dividend_duplicate()
;
