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

CREATE OR REPLACE FUNCTION <schema>.convert_currency
  (
    money   <schema>.eodbar.qclose%TYPE,
    scurr   <schema>.fi.fi_id%TYPE,                         -- Source currency
    dcurr   <schema>.fi.fi_id%TYPE,                         -- Destination currency
    cdate   <schema>.eodbar.qdate%TYPE DEFAULT '3000-01-01' -- CurrencyDate, take last available date
  )
  RETURNS <schema>.eodbar.qclose%TYPE AS
$BODY$

DECLARE
  squote        <schema>.eodbar.qclose%TYPE; -- Source quote
  dquote        <schema>.eodbar.qclose%TYPE; -- Destination quote
  dmoney        <schema>.eodbar.qclose%TYPE; -- Destination money
  query1        TEXT;

BEGIN

  IF scurr = dcurr THEN RETURN money; END IF; -- Nice, nothing todo

  SELECT qclose INTO squote
    FROM <schema>.eodbar
    WHERE fi_id = scurr and qdate = cdate;

  IF squote IS NULL THEN RETURN money; END IF; -- Money is already USD ..or date not found, shit happens

  SELECT qclose INTO dquote
    FROM <schema>.eodbar
    WHERE fi_id = dcurr and qdate = cdate;

  IF dquote IS NULL THEN dquote := 1.0; END IF; --Destination is USD ..or date not found, shit happens

  dmoney := dquote * money/squote;

  RETURN dmoney;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION <schema>.convert_currency
--
