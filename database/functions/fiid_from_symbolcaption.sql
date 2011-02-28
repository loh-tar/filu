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

INSERT INTO <schema>.error(caption, etext) VALUES('SymbolNUQ', 'Symbol references different FIs.');

CREATE OR REPLACE FUNCTION <schema>.fiid_from_symbolcaption
(
  aSymbol <schema>.symbol.caption%TYPE   -- like "AAPL"
)
RETURNS <schema>.fi.fi_id%TYPE AS
$BODY$

DECLARE
  mSymbol    <schema>.symbol.caption%TYPE;
  mFiId      <schema>.fi.fi_id%TYPE;
  mFiId2     <schema>.fi.fi_id%TYPE := -1;

BEGIN

  mSymbol := trim(both from aSymbol);
  IF char_length(mSymbol) = 0 THEN RETURN <schema>.error_code('SymbolEY'); END IF;

  FOR mFiId IN SELECT fi_id
                 FROM <schema>.symbol
                 WHERE lower(caption) LIKE lower(mSymbol)
  LOOP
      IF (mFiId2 = -1) THEN mFiId2 := mFiId; END IF;
      IF mFiId2 <> mFiId THEN RETURN <schema>.error_code('SymbolNUQ'); END IF;

  END LOOP;

  IF mFiId IS NULL THEN RETURN <schema>.error_code('SymbolNF'); END IF;

  RETURN mFiId;

END
$BODY$
LANGUAGE PLPGSQL STABLE;
--
-- END OF FUNCTION <schema>.fiid_from_symbolcaption
--
