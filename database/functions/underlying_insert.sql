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

INSERT INTO <schema>.error(caption, etext) VALUES('MotherSymbolNUQ', 'Mother symbol is not unique.');
INSERT INTO <schema>.error(caption, etext) VALUES('MotherSymbolNF', 'Mother not found.');
INSERT INTO <schema>.error(caption, etext) VALUES('MotherSymbolEY', 'Mother is empty.');
INSERT INTO <schema>.error(caption, etext) VALUES('UlySymbolNUQ', 'Underlying symbol is not unique.');
INSERT INTO <schema>.error(caption, etext) VALUES('UlySymbolNF', 'Underlying not found.');
INSERT INTO <schema>.error(caption, etext) VALUES('UlySymbolEY', 'Underlying is empty.');

CREATE OR REPLACE FUNCTION <schema>.underlying_insert
(
  aMotherSymbol  <schema>.symbol.caption%TYPE,
  aSymbol        <schema>.symbol.caption%TYPE,
  aWeight        <schema>.underlying.weight%TYPE
)
RETURNS int2 AS
$BODY$

DECLARE
  mMotherId        <schema>.fi.fi_id%TYPE;
  mUnderlyingId    <schema>.fi.fi_id%TYPE;

BEGIN
  -- Added the given aSymbol as underlying to the aMotherSymbol.
  -- Returns:
  --    1 if all was fine or <0 if trouble

  mMotherId := <schema>.fiid_from_symbolcaption(aMotherSymbol);
  IF mMotherId < 1 THEN
    IF mMotherId = <schema>.error_code('SymbolNUQ') THEN RETURN <schema>.error_code('MotherSymbolNUQ');
      ELSEIF mMotherId = <schema>.error_code('SymbolNF') THEN RETURN <schema>.error_code('MotherSymbolNF');
      ELSE RETURN <schema>.error_code('MotherSymbolEY');
    END IF;
  END IF;

  mUnderlyingId := <schema>.fiid_from_symbolcaption(aSymbol);
  IF mUnderlyingId < 1 THEN
    IF mMotherId = <schema>.error_code('SymbolNUQ') THEN RETURN <schema>.error_code('UlySymbolNUQ');
      ELSEIF mMotherId = <schema>.error_code('SymbolNF') THEN RETURN <schema>.error_code('UlySymbolNF');
      ELSE RETURN <schema>.error_code('UlySymbolEY');
    END IF;
  END IF;

  INSERT INTO <schema>.underlying(fi_id, underlying_fi_id, weight)
         VALUES(mMotherId, mUnderlyingId, aWeight);

  RETURN 1;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION <schema>.underlying_insert
--
