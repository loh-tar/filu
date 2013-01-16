/*
 *   This file is part of Filu.
 *
 *   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
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

INSERT INTO :filu.error(caption, etext) VALUES('MotherSymbolNUQ', 'Mother symbol is not unique.');
INSERT INTO :filu.error(caption, etext) VALUES('MotherSymbolNF', 'Mother not found.');
INSERT INTO :filu.error(caption, etext) VALUES('MotherSymbolEY', 'Mother is empty.');
INSERT INTO :filu.error(caption, etext) VALUES('UlySymbolNUQ', 'Underlying symbol is not unique.');
INSERT INTO :filu.error(caption, etext) VALUES('UlySymbolNF', 'Underlying not found.');
INSERT INTO :filu.error(caption, etext) VALUES('UlySymbolEY', 'Underlying is empty.');

CREATE OR REPLACE FUNCTION :filu.underlying_insert
(
  aMotherSymbol  :filu.symbol.caption%TYPE,
  aSymbol        :filu.symbol.caption%TYPE,
  aWeight        :filu.underlying.weight%TYPE
)
RETURNS int2 AS
$BODY$

DECLARE
  mMotherFi      :filu.fi.fi_id%TYPE;
  mUndlyFi       :filu.fi.fi_id%TYPE;
  mUndlyId       :filu.underlying.underlying_id%TYPE;

BEGIN
  -- Added the given aSymbol as underlying to the aMotherSymbol.

  mMotherFi := :filu.fiid_from_symbolcaption(aMotherSymbol);
  IF mMotherFi < 1 THEN
    IF mMotherFi = :filu.error_code('SymbolCaptionNUQ') THEN RETURN :filu.error_code('MotherSymbolNUQ');
      ELSEIF mMotherFi = :filu.error_code('SymbolNF') THEN RETURN :filu.error_code('MotherSymbolNF');
      ELSE RETURN :filu.error_code('MotherSymbolEY');
    END IF;
  END IF;

  mUndlyFi := :filu.fiid_from_symbolcaption(aSymbol);
  IF mUndlyFi < 1 THEN
    IF mMotherFi = :filu.error_code('SymbolCaptionNUQ') THEN RETURN :filu.error_code('UlySymbolNUQ');
      ELSEIF mMotherFi = :filu.error_code('SymbolNF') THEN RETURN :filu.error_code('UlySymbolNF');
      ELSE RETURN :filu.error_code('UlySymbolEY');
    END IF;
  END IF;

  -- Check if already associated to mother
  SELECT underlying_id INTO mUndlyId
      FROM :filu.underlying
      WHERE fi_id = mMotherFi and underlying_fi_id = mUndlyFi;

  IF FOUND THEN RETURN mUndlyId; END IF;

  -- Ok, add them
  mUndlyId := nextval(':filu.underlying_underlying_id_seq');
  INSERT INTO :filu.underlying(underlying_id, fi_id, underlying_fi_id, weight)
        VALUES(mUndlyId, mMotherFi, mUndlyFi, aWeight);

  RETURN mUndlyId;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION :filu.underlying_insert
--
