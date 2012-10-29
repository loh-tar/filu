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

INSERT INTO <schema>.error(caption, etext) VALUES('CurryNF', 'Currency not found.');
INSERT INTO <schema>.error(caption, etext) VALUES('FiNameEY', 'FiName is empty.');
INSERT INTO <schema>.error(caption, etext) VALUES('FiNameNUQ', 'FiName is not unique.');

CREATE OR REPLACE FUNCTION <schema>.fi_insert
(
  aFiId   <schema>.fi.fi_id%TYPE,      -- could be 0/NULL
  aFiName <schema>.fi.caption%TYPE,    -- like "Apple Inc."
  aFiType <schema>.ftype.caption%TYPE, -- like "Stock"
  aSymbol <schema>.symbol.caption%TYPE,-- like "AAPL"
  aSType  <schema>.stype.caption%TYPE, -- like "Yahoo"
  aMarket <schema>.market.caption%TYPE -- like "Nyse"
)
RETURNS <schema>.fi.fi_id%TYPE AS
$BODY$

DECLARE
  mFiId       <schema>.fi.fi_id%TYPE;
  mSymbolId   <schema>.symbol.symbol_id%TYPE;
  mFTypeId    <schema>.ftype.ftype_id%TYPE;
  mSTypeId    <schema>.stype.stype_id%TYPE;
  mMarketId   <schema>.market.market_id%TYPE;
  mAddSymbol  bool;

BEGIN
  -- Update the FI or insert with symbol.

  mFiId := COALESCE(aFiId, 0);

  -- Check that the FiType is valid
  mFTypeId := <schema>.id_from_caption('ftype', aFiType);
  IF mFTypeId < 1 THEN RETURN <schema>.error_code('FTypeNF'); END IF;

  mAddSymbol := true;

  IF mFiId = 0 THEN  -- Check if we found the FiName
    mFiId := <schema>.id_from_caption('fi', aFiName);
  END IF;

  IF mFiId < 1 THEN  -- Check if Symbol is known
    mFiId := <schema>.fiid_from_symbolcaption(aSymbol);
    IF mFiId > 0  THEN mAddSymbol := false; END IF; -- Symbol is known
    IF mFiId != <schema>.error_code('SymbolNF') THEN RETURN mFiId; END IF;
  END IF;

  IF mFiId > 0 THEN
    -- quit simple, only update the FI data
    BEGIN
      UPDATE <schema>.fi
        SET caption  = aFiName,
            ftype_id = mFTypeId
        WHERE fi_id = mFiId;

      EXCEPTION WHEN unique_violation THEN RETURN <schema>.error_code('FiNameNUQ');
    END;

    -- So far so good, check the given symbol, if it looks good
    -- add them too
    mSTypeId := <schema>.id_from_caption('stype', aSType);
    IF mSTypeId < 1 THEN mAddSymbol := false; END IF;

    mMarketId := <schema>.id_from_caption('market', aMarket);
    IF mMarketId < 1 THEN mAddSymbol := false; END IF;

    -- Last check if the combination already exist
    SELECT symbol_id INTO mSymbolId
        FROM <schema>.symbol
        WHERE fi_id = mFiId and stype_id = mSTypeId and market_id = mMarketId;

    IF FOUND THEN mAddSymbol := false; END IF;

    IF mAddSymbol THEN
        mSymbolId := <schema>.id_from_caption('symbol', aSymbol);
        IF mSymbolId > 0 THEN RETURN mFiId; END IF; -- Already known, nothing todo

        mSymbolId := nextval('<schema>.symbol_symbol_id_seq');
        INSERT INTO <schema>.symbol(fi_id, caption, stype_id, market_id)
               VALUES(mFiId, aSymbol, mSTypeId, mMarketId);

    END IF;

    RETURN mFiId;
  END IF;

  -- Ok, No mFiId given, Symbol is not known and Name not found
  -- Try to insert both, but make last checks
  mSTypeId := <schema>.id_from_caption('stype', aSType);
  IF mSTypeId < 1 THEN RETURN <schema>.error_code('STypeNF'); END IF;

  mMarketId := <schema>.id_from_caption('market', aMarket);
  IF mMarketId < 1 THEN RETURN <schema>.error_code('MarketNF'); END IF;

  mFiId     := nextval('<schema>.fi_fi_id_seq');
  mSymbolId := nextval('<schema>.symbol_symbol_id_seq');

  INSERT INTO <schema>.fi(fi_id, caption, ftype_id)
          VALUES(mFiId, aFiName, mFTypeId);

  INSERT INTO <schema>.symbol(fi_id, caption, stype_id, market_id)
          VALUES(mFiId, aSymbol, mSTypeId, mMarketId);

  --RAISE INFO 'fi_insert: new FI % with symbol added: %, %', mFiId, aFiName, aSymbol;
  RETURN mFiId;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION <schema>.fi_insert
--
