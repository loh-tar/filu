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

INSERT INTO <schema>.error(caption, etext) VALUES('STypeEY', 'SymbolType is empty.');
--INSERT INTO <schema>.error(caption, etext) VALUES('', '.');

CREATE OR REPLACE FUNCTION <schema>.stype_insert
(
  aSTypeId     <schema>.stype.stype_id%TYPE,
  aCaption     <schema>.stype.caption%TYPE,
  aSeq         <schema>.stype.seq%TYPE,
  aIsProvider  <schema>.stype.isprovider%TYPE
)

RETURNS <schema>.stype.stype_id%TYPE AS

$BODY$

DECLARE
  mExist <schema>.stype.stype_id%TYPE;
  mId    <schema>.stype.stype_id%TYPE;

BEGIN

  mId := COALESCE(aSTypeId, 0);

  IF mId = 0 THEN -- check if mExist
    mId := <schema>.id_from_caption('stype', aCaption);
    IF mId = <schema>.error_code('CaptionEY')
    THEN RETURN <schema>.error_code('STypeEY'); END IF;-- see ftype_insert
  END IF;

  IF mId < 0 THEN
   -- make the insert
    mId := nextval('<schema>.stype_stype_id_seq');
    INSERT INTO <schema>.stype(stype_id, caption, seq, isprovider)
           VALUES(mId, aCaption, aSeq, aIsProvider);
    --RAISE INFO 'stype_insert added %,  %', mId, aCaption;

  ELSE -- make an update
    UPDATE <schema>.stype
      SET
        caption    = aCaption,
        seq        = aSeq,
        isprovider = aIsProvider
      WHERE stype_id = mId;

  END IF;

  RETURN mId;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION <schema>.stype_insert
--
