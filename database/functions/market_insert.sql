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

CREATE OR REPLACE FUNCTION <schema>.market_insert
  (
    market   <schema>.market.caption%TYPE, -- like "Nyse"
    currency <schema>.fi.caption%TYPE,     -- like "US Dollar"
    reuters  <schema>.symbol.caption%TYPE,  -- Reuters currency symbol like "USD"
    marketId <schema>.market.market_id%TYPE = 0
  )
  RETURNS void AS
$BODY$

DECLARE
  fiid      <schema>.fi.fi_id%TYPE;
  symbfiid  <schema>.fi.fi_id%TYPE;
  mid       <schema>.market.market_id%TYPE;
  nomid     <schema>.market.market_id%TYPE;
  query     text;

BEGIN
  -- Creates a new market, the market currency and the currency Reuters symbol.
  -- But it is possible that the currency already exist

  fiid := <schema>.id_from_caption('fi', currency);
  IF fiid > 0
  THEN -- aha, currency name found, only add market
    -- but do one more ckeck to be sure
    query := $$ SELECT fi_id FROM <schema>.symbol
                WHERE LOWER(caption) = LOWER($$ || quote_literal(reuters) || $$); $$;

    EXECUTE query INTO symbfiid;
    IF symbfiid <> fiid
    THEN -- oh-oh...
      RAISE EXCEPTION $$ Currency could not sure identified $$;
      RETURN;
    END IF;

    IF <schema>.id_from_caption('market', market) = 0
    THEN -- make the insert
      mid  := nextval('<schema>.market_market_id_seq');
      INSERT  INTO <schema>.market(market_id, caption, currency_fi_id)
              VALUES(mid, market, fiid);
    ELSE
      RETURN; -- do nothing, market already exist
    END IF;
  ELSE -- we have to create the currency fi, the symbol and the market

    -- in case of very first call 'NoMarket' does not exist
    -- we have to add all dummy stuff
    nomid := <schema>.id_from_caption('market', 'NoMarket');
    IF nomid < 1
    THEN
      fiid := nextval('<schema>.fi_fi_id_seq');
      nomid  := nextval('<schema>.market_market_id_seq');

      INSERT  INTO <schema>.fi(fi_id, caption, ftype_id)
              VALUES(fiid, 'No Currency', <schema>.id_from_caption('ftype', 'Currency'));

      INSERT  INTO <schema>.market(market_id, caption, currency_fi_id)
              VALUES(nomid, 'NoMarket', fiid);

      INSERT  INTO <schema>.symbol(fi_id, caption, stype_id, market_id)
              VALUES(fiid, 'NoCurry'
                    , <schema>.id_from_caption('stype', 'Reuters')
                    , nomid);
    END IF;

    -- add the market, currency fi and the currency symbol
    fiid := nextval('<schema>.fi_fi_id_seq');
    mid  := nextval('<schema>.market_market_id_seq');

    INSERT  INTO <schema>.fi(fi_id, caption, ftype_id)
            VALUES(fiid, currency, <schema>.id_from_caption('ftype', 'Currency'));

    INSERT  INTO <schema>.symbol(fi_id, caption, stype_id, market_id)
            VALUES(fiid, reuters
                      , <schema>.id_from_caption('stype', 'Reuters')
                      , nomid); -- always to NoMarket

    INSERT  INTO <schema>.market(market_id, caption, currency_fi_id)
            VALUES(mid, market, fiid);

  END IF;


END
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION <schema>.market_insert
--
