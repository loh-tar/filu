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

INSERT INTO :filu.error(caption, etext) VALUES('CurryNF', 'Currency not found.');
INSERT INTO :filu.error(caption, etext) VALUES('FiNameEY', 'FiName is empty.');
INSERT INTO :filu.error(caption, etext) VALUES('FiNameNUQ', 'FiName is not unique.');

CREATE OR REPLACE FUNCTION :filu.fi_insert
(
  aFiId   :filu.fi.fi_id%TYPE,      -- could be 0/NULL
  aFiName :filu.fi.caption%TYPE,    -- like "Apple Inc."
  aFiType :filu.ftype.caption%TYPE, -- like "Stock"
  aSymbol :filu.symbol.caption%TYPE,-- like "AAPL"
  aSType  :filu.stype.caption%TYPE, -- like "Yahoo"
  aMarket :filu.market.caption%TYPE -- like "Nyse"
)
RETURNS :filu.fi.fi_id%TYPE AS
$BODY$

DECLARE
  mFiId       :filu.fi.fi_id%TYPE;
  mSymbolId   :filu.symbol.symbol_id%TYPE;
  mFTypeId    :filu.ftype.ftype_id%TYPE;
  mSTypeId    :filu.stype.stype_id%TYPE;
  mMarketId   :filu.market.market_id%TYPE;
  mAddSymbol  bool;

BEGIN
  -- Update the FI or insert with symbol.

  mFiId := COALESCE(aFiId, 0);

  -- Check that the FiType is valid
  mFTypeId := :filu.id_from_caption('ftype', aFiType);
  IF mFTypeId < 1 THEN RETURN :filu.error_code('FTypeNF'); END IF;

  mAddSymbol := true;

  IF mFiId = 0 THEN  -- Check if we found the FiName
    mFiId := :filu.id_from_caption('fi', aFiName);
  END IF;

  IF mFiId < 1 THEN  -- Check if Symbol is known
    mFiId := :filu.fiid_from_symbolcaption(aSymbol);
    IF mFiId > 0  THEN mAddSymbol := false; END IF; -- Symbol is known
    IF mFiId != :filu.error_code('SymbolNF') THEN RETURN mFiId; END IF;
  END IF;

  IF mFiId > 0 THEN
    -- quit simple, only update the FI data
    BEGIN
      UPDATE :filu.fi
        SET caption  = aFiName,
            ftype_id = mFTypeId
        WHERE fi_id = mFiId;

      EXCEPTION WHEN unique_violation THEN RETURN :filu.error_code('FiNameNUQ');
    END;

    -- So far so good, check the given symbol, if it looks good
    -- add them too
    mSTypeId := :filu.id_from_caption('stype', aSType);
    IF mSTypeId < 1 THEN mAddSymbol := false; END IF;

    mMarketId := :filu.id_from_caption('market', aMarket);
    IF mMarketId < 1 THEN mAddSymbol := false; END IF;

    -- Last check if the combination already exist
    SELECT symbol_id INTO mSymbolId
        FROM :filu.symbol
        WHERE fi_id = mFiId and stype_id = mSTypeId and market_id = mMarketId;

    IF FOUND THEN mAddSymbol := false; END IF;

    IF mAddSymbol THEN
        mSymbolId := :filu.id_from_caption('symbol', aSymbol);
        IF mSymbolId > 0 THEN RETURN mFiId; END IF; -- Already known, nothing todo

        mSymbolId := nextval(':filu.symbol_symbol_id_seq');
        INSERT INTO :filu.symbol(fi_id, caption, stype_id, market_id)
               VALUES(mFiId, aSymbol, mSTypeId, mMarketId);

    END IF;

    RETURN mFiId;
  END IF;

  -- Ok, No mFiId given, Symbol is not known and Name not found
  -- Try to insert both, but make last checks
  mSTypeId := :filu.id_from_caption('stype', aSType);
  IF mSTypeId < 1 THEN RETURN :filu.error_code('STypeNF'); END IF;

  mMarketId := :filu.id_from_caption('market', aMarket);
  IF mMarketId < 1 THEN RETURN :filu.error_code('MarketNF'); END IF;

  mFiId     := nextval(':filu.fi_fi_id_seq');
  mSymbolId := nextval(':filu.symbol_symbol_id_seq');

  INSERT INTO :filu.fi(fi_id, caption, ftype_id)
          VALUES(mFiId, aFiName, mFTypeId);

  INSERT INTO :filu.symbol(fi_id, caption, stype_id, market_id)
          VALUES(mFiId, aSymbol, mSTypeId, mMarketId);

  --RAISE INFO 'fi_insert: new FI % with symbol added: %, %', mFiId, aFiName, aSymbol;
  RETURN mFiId;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION :filu.fi_insert
--
