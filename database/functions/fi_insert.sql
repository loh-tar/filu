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

CREATE OR REPLACE FUNCTION <schema>.fi_insert
  (
    nfi_id  <schema>.fi.fi_id%TYPE,      -- could be 0/NULL
    nfiName <schema>.fi.caption%TYPE,    -- like "Apple Inc."
    nfiType <schema>.ftype.caption%TYPE, -- like "Stock"
    nsymbol <schema>.symbol.caption%TYPE,-- like "AAPL"
    nstype  <schema>.stype.caption%TYPE, -- like "Yahoo"
    nmarket <schema>.market.caption%TYPE -- like "Nyse"
  )
  RETURNS <schema>.fi.fi_id%TYPE AS
$BODY$

DECLARE
  fiId        <schema>.fi.fi_id%TYPE;
  sid         <schema>.symbol.symbol_id%TYPE; -- symbol id
  ftypeId     <schema>.ftype.ftype_id%TYPE;
  stId        <schema>.stype.stype_id%TYPE;
  mid         <schema>.market.market_id%TYPE;
  addSymbol   bool;

BEGIN
  -- Update the FI or insert with symbol.
  --
  -- If not the symbol nor the FI name is found, insert both.
  -- If the symbol is found but not the FI name, update the FI name and type.
  -- If a FiId is given, the FI name and type.
  -- Returns:
  --   >0 if all is fine, the fiId
  --   -1 if fiType is not valid
  --   -2 if fiName was empty
  --   -3 if stype is not valid
  --   -4 if market is not valid
  --   -5 if unique violation
  --   -6 if foreign key violation (should impossible?)
  --   -7 if other error (should impossible?)

  fiId := nfi_id;
  IF fiId IS NULL THEN fiId := 0; END IF;

  -- Check that the FiType is valid
  -- -2= empty caption, -1=more than one found, 0=unknown, >1=id
  ftypeId := <schema>.id_from_caption('ftype', nfiType);
  IF ftypeId < 1 THEN RETURN -1; END IF;

  addSymbol := true;

  IF fiId = 0
  THEN  -- Check if we found the FiName
    fiId := <schema>.id_from_caption('fi', nfiName);
  END IF;

  IF fiId < 1
  THEN  -- Check if Symbol is known
    fiId := <schema>.fiid_from_symbolcaption(nsymbol);

    IF fiId > 0 THEN addSymbol :=  false; END IF; -- symbol is known
  END IF;

  IF fiId > 0
  THEN
    -- quit simple, only update the FI data
    UPDATE <schema>.fi
      SET caption  = nfiName,
          ftype_id = ftypeId
      WHERE fi_id = fiId;

    --so far so good, check the given symbol, if it looks good
    -- add them too
    stId := <schema>.id_from_caption('stype', nstype);
    IF stId < 1 THEN addSymbol :=  false; END IF;
    mid := <schema>.id_from_caption('market', nmarket);
    IF mid < 1 THEN addSymbol :=  false; END IF;

    IF addSymbol
    THEN
      BEGIN
        sid := nextval('<schema>.symbol_symbol_id_seq');
        INSERT  INTO <schema>.symbol(fi_id, caption, stype_id, market_id)
          VALUES(fiId, nsymbol, stId, mid);

        EXCEPTION WHEN unique_violation THEN RETURN fiId; -- ignore symbol error
      END;
    END IF;

    RETURN fiId;
  END IF;

  -- Ok, No fiId given, Symbol is not known and Name not found
  -- Try to insert both, but make last checks
  stId := <schema>.id_from_caption('stype', nstype);
  IF stId < 1 THEN  RETURN -3; END IF;
  mid := <schema>.id_from_caption('market', nmarket);
  IF mid < 1 THEN  RETURN -4; END IF;

  fiId := nextval('<schema>.fi_fi_id_seq');
  sid  := nextval('<schema>.symbol_symbol_id_seq');

  BEGIN
    INSERT  INTO <schema>.fi(fi_id, caption, ftype_id)
            VALUES(fiId, nfiName, ftypeId);

    INSERT  INTO <schema>.symbol(fi_id, caption, stype_id, market_id)
            VALUES(fiId, nsymbol, stId, mid);

    --RAISE INFO 'fi_insert: new FI % with symbol added: %, %', fiId, nfiName, nsymbol;
    RETURN fiId;

    EXCEPTION WHEN unique_violation THEN RETURN -5;
              WHEN foreign_key_violation THEN RETURN -6;

  END;

  RETURN -7;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION <schema>.fi_insert
--
