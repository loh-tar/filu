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

CREATE OR REPLACE FUNCTION <schema>.fiid_from_symbolcaption
  (
    symbol <schema>.symbol.caption%TYPE   -- like "AAPL"
  )
  RETURNS <schema>.fi.fi_id%TYPE AS
$BODY$

DECLARE
  mySymbol    <schema>.symbol.caption%TYPE;
  fiId        <schema>.fi.fi_id%TYPE;
  id2         <schema>.fi.fi_id%TYPE;
  query       text;
  myRec       record;

BEGIN
  -- Returns:
  --   >0 if no problem, (the FiId)
  --    0 if Symbol was not found
  --   -1 if Symbol was found more than one time and was associated to different FIs
  --   -2 if Symbol was empty

  mySymbol := trim(both from symbol);
  IF char_length(mySymbol) = 0 THEN RETURN -2; END IF;

  query := $$ SELECT fi_id FROM <schema>.symbol
              WHERE LOWER(caption) LIKE LOWER($$ || quote_literal(mySymbol) || $$)$$;

  id2 := -1;
  FOR myRec IN EXECUTE query LOOP
      fiId := myRec.fi_id;
      IF (id2 = -1)  THEN id2 := fiId; END IF;
      IF id2 <> fiId THEN RETURN -1; END IF;

  END LOOP;

  IF fiId IS NULL THEN RETURN 0; END IF;

  RETURN fiId;

END
$BODY$
  LANGUAGE 'plpgsql' STABLE;
--
-- END OF FUNCTION <schema>.fiid_from_symbolcaption
--
