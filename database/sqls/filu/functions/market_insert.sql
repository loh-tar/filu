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

INSERT INTO :filu.error(caption, etext) VALUES('MarketIdNF', 'MarketId not found.');
INSERT INTO :filu.error(caption, etext) VALUES('MarketNF', 'Market not found.');
--INSERT INTO :filu.error(caption, etext) VALUES('', '');

CREATE OR REPLACE FUNCTION :filu.market_insert
(
  aMarket   :filu.market.caption%TYPE, -- like "Nyse"
  aCurrency :filu.fi.caption%TYPE,     -- like "US Dollar"
  aSymbol   :filu.symbol.caption%TYPE, -- Symbol of the currency provider, like "USD"
  aMarketId :filu.market.market_id%TYPE = 0
)
RETURNS :filu.market.market_id%TYPE AS
$BODY$

DECLARE
  mFiId      :filu.fi.fi_id%TYPE;
  mMId       :filu.market.market_id%TYPE; -- Market ID
  mNoMId     int := 1;                    -- NoMarket has always ID 1
  mForexId   int := 2;                    -- Forex has always ID 2
  mCpId      int := 1;                    -- CurrencyProviderId has always ID 1
  mNumRows   int;

BEGIN

  mFiId := :filu.fiid_from_symbolcaption(aSymbol);
  IF mFiId < 1 THEN mFiId := :filu.id_from_caption('fi', aCurrency); END IF;

  IF aMarketId > 0 THEN
    -- Ok, looks easy only update
    IF aMarketId = mNoMId THEN RETURN mNoMId; END IF; -- Don't change NoMarket!
    IF mFiId < 1 THEN RETURN :filu.error_code('CurryNF'); END IF;

    UPDATE :filu.market
      SET caption = aMarket, currency_fi_id = mFiId
      WHERE market_id = aMarketId;

    GET DIAGNOSTICS mNumRows = ROW_COUNT;
    IF mNumRows > 0 THEN RETURN aMarketId; ELSE RETURN :filu.error_code('MarketIdNF'); END IF;

  END IF;

  IF mFiId > 0 THEN
    -- Aha, currency found, only add market
    mMId := :filu.id_from_caption('market', aMarket);
    IF mMId < 1 THEN
      -- make the insert
      mMId  := nextval(':filu.market_market_id_seq');
      INSERT  INTO :filu.market(market_id, caption, currency_fi_id)
              VALUES(mMId, aMarket, mFiId);
    END IF;

    RETURN mMId;

  ELSE -- we have to create the currency fi, the symbol and the market

    IF aMarketId < 0 THEN
      -- in case of very first call 'NoMarket' does not exist
      -- we have to add all dummy stuff
      mFiId  := nextval(':filu.fi_fi_id_seq');
      mNoMId := nextval(':filu.market_market_id_seq');
      -- Change the symbol type, it is pointless to try to download data for the base currency
      mCpId    := :filu.id_from_caption('stype', 'Reuters');
      mForexId := mNoMId;

      INSERT  INTO :filu.fi(fi_id, caption, ftype_id)
              VALUES(mFiId, 'No Currency', :filu.id_from_caption('ftype', 'Currency'));

      INSERT  INTO :filu.market(market_id, caption, currency_fi_id)
              VALUES(mNoMId, 'NoMarket', mFiId);

      INSERT  INTO :filu.symbol(fi_id, caption, stype_id, market_id)
              VALUES(mFiId, 'NoCurry'
                    , mCpId   -- The variable is here abused
                    , mNoMId);

      --RAISE INFO 'Dummy stuff NoMarket and NoCurrency had been created.';
    END IF;

    IF aCurrency IS NULL THEN RETURN :filu.error_code('FiNameEY'); END IF;
    IF char_length(trim(both from aCurrency)) = 0 THEN RETURN :filu.error_code('FiNameEY'); END IF;

    -- add the market, currency fi and the currency symbol
    mFiId := nextval(':filu.fi_fi_id_seq');
    mMId  := nextval(':filu.market_market_id_seq');

    INSERT  INTO :filu.fi(fi_id, caption, ftype_id)
            VALUES(mFiId, aCurrency, :filu.id_from_caption('ftype', 'Currency'));

    INSERT  INTO :filu.market(market_id, caption, currency_fi_id)
            VALUES(mMId, aMarket, mFiId);

    INSERT  INTO :filu.symbol(fi_id, caption, stype_id, market_id)
            VALUES(mFiId, aSymbol, mCpId, mForexId);

  END IF;

  RETURN mMId;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION :filu.market_insert
--
