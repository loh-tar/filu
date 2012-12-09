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

CREATE OR REPLACE FUNCTION :filu.convert_currency
(
  aMoney   :filu.eodbar.qclose%TYPE,
  aSCurr   :filu.fi.fi_id%TYPE,                         -- Source currency
  aDCurr   :filu.fi.fi_id%TYPE,                         -- Destination currency
  aCDate   :filu.eodbar.qdate%TYPE DEFAULT '3000-01-01' -- CurrencyDate, take last available date
)
RETURNS :filu.eodbar.qclose%TYPE AS
$BODY$

DECLARE
  mSQuote       :filu.eodbar.qclose%TYPE; -- Source quote
  mDQuote       :filu.eodbar.qclose%TYPE; -- Destination quote
  mDMoney       :filu.eodbar.qclose%TYPE; -- Destination aMoney
  mUSDollar     CONSTANT int := 2;        -- US Dollar has always ID 2

BEGIN
  --
  -- Returns (Money * -1) if no (source or destination) quote found
  --

  IF aSCurr = aDCurr THEN RETURN aMoney; END IF; -- Nice, nothing todo

  IF aSCurr = mUSDollar
    THEN mSQuote = 1.0;
    ELSE SELECT qclose INTO mSQuote
           FROM :filu.eodbar
           WHERE fi_id = aSCurr and qdate <= aCDate
           ORDER BY qdate DESC LIMIT 1;

          IF mSQuote IS NULL THEN RETURN -aMoney; END IF; -- Currency or date not found, shit happens
  END IF;

  IF aDCurr = mUSDollar
    THEN mDQuote = 1.0;
    ELSE SELECT qclose INTO mDQuote
           FROM :filu.eodbar
           WHERE fi_id = aDCurr and qdate <= aCDate
           ORDER BY qdate DESC LIMIT 1;

          IF mDQuote IS NULL THEN RETURN -aMoney; END IF; -- Currency or date not found, shit happens
  END IF;

  mDMoney := mSQuote * aMoney/mDQuote;

  RETURN mDMoney;

END;
$BODY$
LANGUAGE PLPGSQL STABLE;
--
-- END OF FUNCTION :filu.convert_currency
--
