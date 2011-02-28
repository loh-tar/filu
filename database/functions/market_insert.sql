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

INSERT INTO <schema>.error(caption, etext) VALUES('MarketIdNF', 'MarketId not found.');
INSERT INTO <schema>.error(caption, etext) VALUES('MarketNF', 'Market not found.');
--INSERT INTO <schema>.error(caption, etext) VALUES('', '');

CREATE OR REPLACE FUNCTION <schema>.market_insert
(
  aMarket   <schema>.market.caption%TYPE, -- like "Nyse"
  aCurrency <schema>.fi.caption%TYPE,     -- like "US Dollar"
  aReuters  <schema>.symbol.caption%TYPE, -- Reuters currency symbol like "USD"
  aMarketId <schema>.market.market_id%TYPE = 0
)
RETURNS <schema>.market.market_id%TYPE AS
$BODY$

DECLARE
  mFiId      <schema>.fi.fi_id%TYPE;
  mMId       <schema>.market.market_id%TYPE; -- Market ID
  mNoMId     int := 1;                       -- NoMarket has always ID 1
  mNumRows   int;

BEGIN

  mFiId := fiid_from_symbolcaption(aReuters);
  IF mFiId < 1 THEN mFiId := <schema>.id_from_caption('fi', aCurrency); END IF;

  IF aMarketId > 0 THEN
    -- Ok, looks easy only update
    IF aMarketId = mNoMId THEN RETURN mNoMId; END IF; -- Don't change NoMarket!
    IF mFiId < 1 THEN RETURN <schema>.error_code('CurryNF'); END IF;

    UPDATE <schema>.market
      SET caption = aMarket, currency_fi_id = mFiId
      WHERE market_id = aMarketId;

    GET DIAGNOSTICS mNumRows = ROW_COUNT;
    IF mNumRows > 0 THEN RETURN aMarketId; ELSE RETURN <schema>.error_code('MarketIdNF'); END IF;

  END IF;

  IF mFiId > 0 THEN
    -- Aha, currency found, only add market
    mMId := <schema>.id_from_caption('market', aMarket);
    IF mMId < 1 THEN
      -- make the insert
      mMId  := nextval('<schema>.market_market_id_seq');
      INSERT  INTO <schema>.market(market_id, caption, currency_fi_id)
              VALUES(mMId, aMarket, mFiId);
    END IF;

    RETURN mMId;

  ELSE -- we have to create the currency fi, the symbol and the market

    IF aMarketId < 0 THEN
      -- in case of very first call 'NoMarket' does not exist
      -- we have to add all dummy stuff
      mFiId  := nextval('<schema>.fi_fi_id_seq');
      mNoMId := nextval('<schema>.market_market_id_seq');

      INSERT  INTO <schema>.fi(fi_id, caption, ftype_id)
              VALUES(mFiId, 'No Currency', <schema>.id_from_caption('ftype', 'Currency'));

      INSERT  INTO <schema>.market(market_id, caption, currency_fi_id)
              VALUES(mNoMId, 'NoMarket', mFiId);

      INSERT  INTO <schema>.symbol(fi_id, caption, stype_id, market_id)
              VALUES(mFiId, 'NoCurry'
                    , <schema>.id_from_caption('stype', 'Reuters')
                    , mNoMId);

      RAISE INFO 'Dummy stuff NoMarket and NoCurrency had been created.';
    END IF;

    IF aCurrency IS NULL THEN RETURN <schema>.error_code('FiNameEY'); END IF;
    IF char_length(trim(both from aCurrency)) = 0 THEN RETURN <schema>.error_code('FiNameEY'); END IF;

    -- add the market, currency fi and the currency symbol
    mFiId := nextval('<schema>.fi_fi_id_seq');
    mMId  := nextval('<schema>.market_market_id_seq');

    INSERT  INTO <schema>.fi(fi_id, caption, ftype_id)
            VALUES(mFiId, aCurrency, <schema>.id_from_caption('ftype', 'Currency'));

    INSERT  INTO <schema>.symbol(fi_id, caption, stype_id, market_id)
            VALUES(mFiId, aReuters
                      , <schema>.id_from_caption('stype', 'Reuters')
                      , mNoMId); -- always to NoMarket

    INSERT  INTO <schema>.market(market_id, caption, currency_fi_id)
            VALUES(mMId, aMarket, mFiId);

  END IF;

  RETURN mMId;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION <schema>.market_insert
--
