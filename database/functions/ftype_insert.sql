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

INSERT INTO <schema>.error(caption, etext) VALUES('FTypeNF', 'FiType not found.');
INSERT INTO <schema>.error(caption, etext) VALUES('FTypeNUQ', 'FiType already exist, give me the ID for an update.');

CREATE OR REPLACE FUNCTION <schema>.ftype_insert
(
  aCaption  <schema>.ftype.caption%TYPE,
  aFTypeId  <schema>.ftype.ftype_id%TYPE-- could be 0/NULL
)
RETURNS <schema>.ftype.ftype_id%TYPE AS
$BODY$

DECLARE
  mId       <schema>.ftype.ftype_id%TYPE; -- New ID

BEGIN

  mId := COALESCE(aFTypeId, 0);

  IF mId = 0 THEN
    mId := <schema>.id_from_caption('ftype', aCaption);
    IF mId > 0 THEN RETURN mId/*<schema>.error_code('FTypeNUQ')*/; END IF;-- see stype_insert

    mId := nextval('<schema>.ftype_ftype_id_seq');
    INSERT  INTO <schema>.ftype(ftype_id, caption)
            VALUES(mId, aCaption);

    RETURN mId;
  END IF;

  UPDATE <schema>.ftype
    SET caption    = aCaption
    WHERE ftype_id = aFTypeId;

  IF FOUND THEN RETURN aFTypeId; END IF;

  RETURN <schema>.error_code('PrimaryKeyNF');

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION <schema>.ftype_insert
--
