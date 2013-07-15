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

CREATE OR REPLACE FUNCTION :filu.gfi_xcurrency_eodbar
(
  aFiId        :filu.fi.fi_id%TYPE,
  aMarketId    :filu.market.market_id%TYPE,
  aFDate       :filu.eodbar.qdate%TYPE       DEFAULT '1000-01-01',
  aTDate       :filu.eodbar.qdate%TYPE       DEFAULT '3000-01-01',
  aLimit       int4                          DEFAULT 0 -- max number of rows
)
RETURNS SETOF :filu.fbar AS
$BODY$
DECLARE
  mRecord       record;
  mBaseCurr     CONSTANT int := 2;           -- Base currency (of our Forex market) has always ID 2
  mCurr1        :filu.fi.fi_id%TYPE;         -- see also fetchBar_ECB.pl
  mCurr2        :filu.fi.fi_id%TYPE;
  mHelp         TEXT[];
  mResult       :filu.fbar;

BEGIN
  --
  -- Fetch both currencys...
  -- The symbols at the begining of the FI caption e.g. GBP/USD => mCurr1/mCurr2
  --
  SELECT caption INTO mRecord FROM :filu.fi WHERE fi_id = aFiId;
  IF mRecord IS NULL THEN RETURN; END IF;

  mHelp := regexp_split_to_array(mRecord.caption, E'\\/');
  mCurr1 := :filu.fiid_from_symbolcaption(mHelp[1]);
  IF mCurr1 < 1 THEN RETURN; END IF;

  mHelp := regexp_split_to_array(mHelp[2], E'\\s');
  mCurr2 := :filu.fiid_from_symbolcaption(mHelp[1]);
  IF mCurr2 < 1 THEN RETURN; END IF;

  IF mCurr2 = mBaseCurr THEN -- GBP/EUR
    RETURN QUERY SELECT * FROM :filu.gfi_rawdata_eodbar(mCurr1, aMarketId, aFDate, aTDate, aLimit);
  END IF;

  IF mCurr1 = mBaseCurr THEN -- EUR/GBP
    FOR mRecord IN
        SELECT * FROM :filu.gfi_rawdata_eodbar(mCurr2, aMarketId, aFDate, aTDate, aLimit)
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

    --  GBP/USD
    FOR mRecord IN
        SELECT fdate, ftime, c1.fclose AS close1, c2.fclose AS close2
          FROM :filu.gfi_rawdata_eodbar(mCurr1, aMarketId, aFDate, aTDate, aLimit) AS c1
          JOIN :filu.gfi_rawdata_eodbar(mCurr2, aMarketId, aFDate, aTDate, aLimit) AS c2
          USING (fdate, ftime)

    LOOP
      mResult.fdate  := mRecord.fdate;
      mResult.ftime  := mRecord.ftime;

      -- Not only because ECB delivers no bars but a single quote, we set all to the same value.
      -- Would it be right to calc open,high,low if we had bars? I'm in doubt
      --
      -- Unlike in fetchBar_ECB.pl we have not to calc close2/close1 because
      -- the quotes in the DB are inverted from the original data by the ECB
      mResult.fopen  := mRecord.close1 / mRecord.close2;
      mResult.fhigh  := mRecord.close1 / mRecord.close2;
      mResult.flow   := mRecord.close1 / mRecord.close2;
      mResult.fclose := mRecord.close1 / mRecord.close2;

      mResult.fvol   := 0.0;
      mResult.foi    := 0.0;

      RETURN NEXT mResult;
    END LOOP;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;

INSERT INTO :filu.ftype(caption) VALUES('XCurrency');

--
-- END OF FUNCTION :filu.gfi_currency_eodbar(...)
--
