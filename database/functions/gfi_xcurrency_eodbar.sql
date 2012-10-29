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

CREATE OR REPLACE FUNCTION <schema>.gfi_xcurrency_eodbar
(
  aFiId        <schema>.fi.fi_id%TYPE,
  aMarketId    <schema>.market.market_id%TYPE,
  aFDate       <schema>.eodbar.qdate%TYPE       DEFAULT '1000-01-01',
  aTDate       <schema>.eodbar.qdate%TYPE       DEFAULT '3000-01-01',
  aLimit       int4                             DEFAULT 0 -- max number of rows
)
RETURNS SETOF <schema>.fbar AS
$BODY$
DECLARE
  mRecord       record;
  mCurryMarket  CONSTANT int := 1;
  mUSDollar     CONSTANT int := 2;           -- US Dollar has always ID 2
  mSCurrId      <schema>.fi.fi_id%TYPE;
  mDCurrId      <schema>.fi.fi_id%TYPE;
  mSymbol       <schema>.symbol.caption%TYPE;
  mHelp         TEXT[];
  mResult       <schema>.fbar;

BEGIN
  --
  -- Fetch both currencys...
  -- The symbols at the begining of the FI caption e.g. "USD/EUR"
  --
  SELECT caption INTO mRecord FROM <schema>.fi WHERE fi_id = aFiId;
  IF mRecord IS NULL THEN RETURN; END IF;

  mHelp := regexp_split_to_array(mRecord.caption, E'\\/');
  mDCurrId := <schema>.fiid_from_symbolcaption(mHelp[1]);
  IF mDCurrId < 1 THEN RETURN; END IF;

  mHelp := regexp_split_to_array(mHelp[2], E'\\s');
  mSCurrId := <schema>.fiid_from_symbolcaption(mHelp[1]);
  IF mSCurrId < 1 THEN RETURN; END IF;

  IF mSCurrId = mUSDollar THEN
    RETURN QUERY SELECT * FROM <schema>.gfi_rawdata_eodbar(mDCurrId, aMarketId, aFDate, aTDate, aLimit);
  END IF;


  IF mDCurrId = mUSDollar THEN
    FOR mRecord IN
        SELECT * FROM <schema>.gfi_rawdata_eodbar(mSCurrId, aMarketId, aFDate, aTDate, aLimit)
    LOOP
      mResult.fdate  := mRecord.fdate;
      mResult.ftime  := mRecord.ftime;

      mResult.fopen  := 1.0 / mRecord.fopen;
      mResult.fhigh  := 1.0 / mRecord.fhigh;
      mResult.flow   := 1.0 / mRecord.flow;
      mResult.fclose := 1.0 / mRecord.fclose;
      mResult.fvol   := 0.0;

      mResult.foi    := 0.0;

      RETURN NEXT mResult;
    END LOOP;
  END IF;


--  mDMoney := mDQuote * aMoney/mSQuote;


END
$BODY$
LANGUAGE PLPGSQL VOLATILE;

INSERT INTO <schema>.ftype(caption) VALUES('XCurrency');

--
-- END OF FUNCTION <schema>.gfi_currency_eodbar(...)
--
