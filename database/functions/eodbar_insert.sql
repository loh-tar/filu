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

-- insert / update function
CREATE OR REPLACE FUNCTION <schema>.eodbar_insert
(   aFiId       <schema>.fi.fi_id%TYPE
  , aMarketId   <schema>.market.market_id%TYPE
  , aDate       <schema>.eodbar.qdate%TYPE
  , aOpen       <schema>.eodbar.qopen%TYPE
  , aHigh       <schema>.eodbar.qhigh%TYPE
  , aLow        <schema>.eodbar.qlow%TYPE
  , aClose      <schema>.eodbar.qclose%TYPE
  , aVol        <schema>.eodbar.qvol%TYPE
  , aOi         <schema>.eodbar.qoi%TYPE        DEFAULT 0
  , aQuality    <schema>.eodbar.quality%TYPE    DEFAULT 2 -- bronze, as tempo classified data
)
RETURNS void AS
$BODY$

DECLARE
  mOpen         <schema>.eodbar.qopen%TYPE;
  mHigh         <schema>.eodbar.qhigh%TYPE;
  mLow          <schema>.eodbar.qlow%TYPE;
  mClose        <schema>.eodbar.qclose%TYPE;
  mVol          <schema>.eodbar.qvol%TYPE;
  mOi           <schema>.eodbar.qoi%TYPE;
  mQuality      <schema>.eodbar.quality%TYPE;

  mExist        <schema>.eodbar.eodbar_id%TYPE;
  mQualyExist   <schema>.eodbar.quality%TYPE;

BEGIN

  mClose   := COALESCE(aClose, 0);
  mOpen    := COALESCE(aOpen, mClose);
  mHigh    := COALESCE(aHigh, mClose);
  mLow     := COALESCE(aLow,  mClose);
  mVol     := COALESCE(aVol, 0);
  mOi      := COALESCE(aOi, 0);
  mQuality := COALESCE(aQuality, 2);

  -- Check plausibility, especially yahoo tends to bugs in eod data
  IF mClose = 0.0
  THEN RAISE NOTICE 'fiId %, %: O:% H:% L:% C:%, close is 0', aFiId, aDate, aOpen, aHigh, aLow, aClose;
        END IF;

  IF mHigh < mClose
  THEN RAISE NOTICE 'fiId %, %: O:% H:% L:% C:%, close was > high', aFiId, aDate, aOpen, aHigh, aLow, aClose;
        mHigh := mClose;
        mQuality := 3;
        END IF;

  IF mClose < mLow
  THEN RAISE NOTICE 'fiId %, %: O:% H:% L:% C:%, close was < low', aFiId, aDate, aOpen, aHigh, aLow, aClose;
        mLow := mClose;
        mQuality := 3;
        END IF;

  IF mOpen > mHigh
  THEN RAISE NOTICE 'fiId %, %: O:% H:% L:% C:%, open was > high', aFiId, aDate, aOpen, aHigh, aLow, aClose;
        mHigh := mOpen;
        mQuality := 3;
        END IF;

  IF mOpen < mLow
  THEN RAISE NOTICE 'fiId %, %: O:% H:% L:% C:%, open was < low', aFiId, aDate, aOpen, aHigh, aLow, aClose;
        mLow := mOpen;
        mQuality := 3;
        END IF;

  SELECT eodbar_id, quality INTO mExist, mQualyExist
      FROM <schema>.eodbar
      WHERE fi_id = aFiId and market_id = aMarketId and qdate = aDate;

  IF FOUND THEN -- make an update
    IF mQualyExist < mQuality
    THEN
      --RAISE NOTICE 'Quality to bad.';
      RETURN;
    END IF; -- quality  to bad

    if aVol = -1 -- in case of an update with bar data where no volume exist, like from onvista
    then
      update <schema>.eodbar
      set
        qopen          = mOpen,
        qhigh          = mHigh,
        qlow           = mLow,
        qclose         = mClose,
        --qvol           = mVol,
        qoi            = mOi,
        quality        = mQuality
      where eodbar_id = mExist;
    else
      update <schema>.eodbar
      set
        qopen          = mOpen,
        qhigh          = mHigh,
        qlow           = mLow,
        qclose         = mClose,
        qvol           = mVol,
        qoi            = mOi,
        quality        = mQuality
      where eodbar_id = mExist;
    end if;

    RETURN;
  END IF;

  INSERT INTO <schema>.eodbar(fi_id, market_id, qdate, qopen, qhigh, qlow, qclose, qvol, qoi, quality)
          VALUES(aFiId, aMarketId, aDate, mOpen, mHigh, mLow, mClose, mVol, mOi, mQuality);

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;

--
-- END OF FUNCTION <schema>.eodbar_insert
--
