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

-- unshure if the right way...still use stype_duplicates() trigger
CREATE OR REPLACE FUNCTION <schema>.stype_insert
(
  nstype_id    <schema>.stype.stype_id%TYPE,
  ncaption     <schema>.stype.caption%TYPE,
  nseq         <schema>.stype.seq%TYPE,
  nisprovider  <schema>.stype.isprovider%TYPE
)

RETURNS <schema>.stype.stype_id%TYPE AS

$BODY$

DECLARE
  exist <schema>.stype.stype_id%TYPE;
  nid   <schema>.stype.stype_id%TYPE;

BEGIN
  -- Update or insert the SymbolType
  --
  -- Returns:
  --   >0 if all is fine, the nstype_id
  --   -1 if caption was empty

  nid := nstype_id;
  IF nstype_id = NULL THEN nid := 0; END IF;

  IF nid = 0
  THEN -- check if exist
    -- -2= empty caption, -1=more than one found, 0=unknown, >1=id
    nid := <schema>.id_from_caption('stype', ncaption);
    IF nid < 0 THEN RETURN -1; END IF;
  END IF;

  IF nid = 0
  THEN
   -- make the insert
    nid := nextval('<schema>.stype_stype_id_seq');
    INSERT INTO <schema>.stype(stype_id, caption, seq, isprovider)
            VALUES(nid, ncaption, nseq, nisprovider);
    --RAISE INFO 'stype_insert added %,  %', nid, ncaption;

  ELSE -- make an update
    UPDATE <schema>.stype
      SET
        caption    = ncaption,
        seq        = nseq,
        isprovider = nisprovider
      WHERE stype_id = nid;

  END IF;

  RETURN nid;

END;
$BODY$
LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION <schema>.stype_insert
--
