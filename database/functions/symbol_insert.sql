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

--INSERT INTO <schema>.error(caption, etext) VALUES('', '.');

CREATE OR REPLACE FUNCTION <schema>.symbol_insert
(
  aFiId     <schema>.fi.fi_id%TYPE,
  aSymbolId <schema>.symbol.symbol_id%TYPE,
  aSymbol   <schema>.symbol.caption%TYPE, -- like "AAPL"
  aMarket   <schema>.market.caption%TYPE, -- like "NASDAQ"
  aSType    <schema>.stype.caption%TYPE   -- like "Yahoo"
)
RETURNS <schema>.symbol.symbol_id%TYPE AS
$BODY$

DECLARE
  mSymbol    <schema>.symbol.caption%TYPE;
  mSymbolId  <schema>.symbol.symbol_id%TYPE;
  mSTypeId   <schema>.stype.stype_id%TYPE;
  mMarketId  <schema>.market.market_id%TYPE;

BEGIN

  mSymbol := trim(both from aSymbol);
  IF char_length(mSymbol) = 0 THEN RETURN <schema>.error_code('SymbolEY'); END IF; -- aSymbol caption empty, pointless

  mSTypeId := <schema>.id_from_caption('stype', aSType);
  IF mSTypeId < 1 THEN RETURN <schema>.error_code('STypeNF'); END IF;

  mMarketId  := <schema>.id_from_caption('market', aMarket);
  IF mMarketId < 1 THEN RETURN <schema>.error_code('MarketNF'); END IF;

  IF aFiId < 1 THEN -- aFiId is a must have, we don't add or update, only look if exist
    mSymbolId := <schema>.id_from_caption('symbol', mSymbol);
    IF mSymbolId = <schema>.error_code('CaptionNUQ') THEN --ops, really rare, aSymbol exist more than one time.
      SELECT INTO mSymbolId symbol_id
        FROM <schema>.symbol
        WHERE
          lower(caption) = lower(aSymbol)
          and market_id = mMarketId
          and stype_id  = mSTypeId;

      IF mSymbolId IS NULL THEN RETURN <schema>.error_code('SymbolNUQ'); END IF;

    END IF; -- mSymbolId = -1

    RETURN mSymbolId;

  END IF; -- aFiId < 1

  mSymbolId := aSymbolId;

  IF mSymbolId < 1 THEN -- no aSymbolId given, we have to check if the symbol is already known
     mSymbolId := <schema>.id_from_caption('symbol', mSymbol);

    IF mSymbolId = <schema>.error_code('CaptionNUQ') THEN --ops, really rare, aSymbol exist more than one time.
      SELECT INTO mSymbolId symbol_id
        FROM <schema>.symbol
        WHERE
          lower(caption) = lower(mSymbol)
          and market_id = mMarketId
          and stype_id  = mSTypeId;

    END IF;

    IF (mSymbolId < 0) or (mSymbolId IS NULL) THEN -- insert the new symbol
      mSymbolId := nextval('<schema>.symbol_symbol_id_seq');
      BEGIN
        INSERT INTO <schema>.symbol(symbol_id, market_id, stype_id, fi_id, caption)
               VALUES(mSymbolId, mMarketId, mSTypeId, aFiId, mSymbol);

        RETURN mSymbolId;

        EXCEPTION WHEN unique_violation THEN RETURN <schema>.error_code('UniqueV');
                  WHEN foreign_key_violation THEN RETURN <schema>.error_code('ForeignKV');
      END;

    END IF;
  END IF; -- mSymbolId < 1

  BEGIN
    -- ok, we have to update the symbol
    UPDATE <schema>.symbol
      SET
        market_id    = mMarketId,
        stype_id     = mSTypeId,
        caption      = mSymbol,
        fi_id        = aFiId
        --issuedate    = issuedate,
        --maturitydate = maturitydate
      WHERE
        symbol_id = mSymbolId;

      RETURN mSymbolId;

    EXCEPTION WHEN unique_violation THEN RETURN <schema>.error_code('UniqueV');

  END;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION <schema>.symbol_insert
--
