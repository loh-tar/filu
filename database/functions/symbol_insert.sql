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

CREATE OR REPLACE FUNCTION <schema>.symbol_insert
  (
    fiId     <schema>.fi.fi_id%TYPE,
    symbolId <schema>.symbol.symbol_id%TYPE,
    symbol   <schema>.symbol.caption%TYPE, -- like "AAPL"
    market   <schema>.market.caption%TYPE, -- like "NASDAQ"
    stype    <schema>.stype.caption%TYPE   -- like "Yahoo"
  )
  RETURNS <schema>.symbol.symbol_id%TYPE AS
$BODY$

DECLARE
  msymbol   <schema>.symbol.caption%TYPE;   -- my symbol (a copy)
  msymbolId <schema>.symbol.symbol_id%TYPE; -- my symbolId (a copy)
  stypeId   <schema>.stype.stype_id%TYPE;
  marketId  <schema>.market.market_id%TYPE;

BEGIN
  -- Insert, update or check the given Symbol.
  -- Returns:
  --   >0 the symbolId, if Symbol exist
  --    0 if Symbol looks good but no fiId was given to add them
  --   -1 SymbolType not valid
  --   -2 Market not valid
  --   -3 unique violation
  --   -4 foreign key violation

  msymbol := trim(both from symbol);
  IF char_length(msymbol) = 0 THEN RETURN -1; END IF; -- symbol caption empty, pointless

  -- -2= empty caption, -1=more than one found, 0=unknown, >1=id
  stypeId := <schema>.id_from_caption('stype', stype);
  IF stypeId < 1 THEN RETURN -1; END IF;

  marketId  := <schema>.id_from_caption('market', market);
  IF marketId < 1 THEN RETURN -2; END IF;

  IF fiId < 1
  THEN -- fiId is a must have, we don't add or update, only look if exist
    msymbolId := <schema>.id_from_caption('symbol', msymbol);
    IF msymbolId = -1
    THEN --ops, really rare, symbol exist more than one time.
      SELECT INTO msymbolId symbol_id
        FROM <schema>.symbol
        WHERE
          LOWER(caption) = LOWER(symbol)
          AND market_id = marketId
          AND stype_id  = stypeId;

      IF msymbolId IS NULL THEN RETURN 0; END IF;

    END IF; -- msymbolId = -1

    RETURN msymbolId;

  END IF; -- fiId < 1

  msymbolId := symbolId;

  IF msymbolId < 1
  THEN -- no symbolId given, we have to check if the symbol is already known
     msymbolId := <schema>.id_from_caption('symbol', msymbol);

    IF msymbolId = -1
    THEN --ops, really rare, symbol exist more than one time.
      SELECT INTO msymbolId symbol_id
        FROM <schema>.symbol
        WHERE
          LOWER(caption) = LOWER(symbol)
          AND market_id = marketId
          AND stype_id  = stypeId;

    END IF;

    IF (msymbolId = 0) OR (msymbolId IS NULL)
    THEN -- insert the new symbol
      msymbolId := nextval('<schema>.symbol_symbol_id_seq');
      BEGIN
        INSERT  INTO <schema>.symbol(symbol_id, market_id, stype_id, fi_id, caption)
                VALUES(msymbolId, marketId, stypeId, fiId, msymbol);

        RETURN msymbolId;

        EXCEPTION WHEN unique_violation THEN RETURN -3;
                  WHEN foreign_key_violation THEN RETURN -4;

      END;

    END IF;
  END IF; -- msymbolId < 1

  BEGIN
    -- ok, we have to update the symbol
    UPDATE <schema>.symbol
      SET
        market_id    = marketId,
        stype_id     = stypeId,
        caption      = msymbol,
        fi_id        = fiId
        --issuedate    = issuedate,
        --maturitydate = maturitydate
      WHERE
        symbol_id = msymbolId;

      RETURN msymbolId;

    EXCEPTION WHEN unique_violation THEN RETURN -3;

  END;

END
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION <schema>.symbol_insert
--
