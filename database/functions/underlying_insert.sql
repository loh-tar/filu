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

CREATE OR REPLACE FUNCTION <schema>.underlying_insert
  (
    motherSymbol  varchar,
    symbol        varchar,
    newWeight     float
  )
  RETURNS int2 AS
$BODY$

DECLARE
  motherId        <schema>.fi.fi_id%TYPE;
  underlyingId    <schema>.fi.fi_id%TYPE;

BEGIN
  -- Added the given symbol to as underlying to the motherSymbol.
  -- Returns:
  --    1 if all was fine
  --   -1 if mother not found
  --   -2 if mother exist more than one times
  --   -3 if underlying not found
  --   -4 if underlying exist more than one times

  -- -2= empty caption, -1=more than one found and was associated to different FIs, 0=unknown, >1=id
  motherId := <schema>.fiid_from_symbolcaption(motherSymbol);
  IF (motherId =  0) OR (motherId = -2 ) THEN RETURN -1; END IF;
  IF (motherId = -1) THEN RETURN -2; END IF;

  underlyingId := <schema>.fiid_from_symbolcaption(symbol);
  IF (underlyingId =  0) OR (underlyingId = -2 ) THEN RETURN -3; END IF;
  IF (underlyingId = -1) THEN RETURN -4; END IF;

  INSERT  INTO <schema>.underlying(fi_id, underlying_fi_id, weight)
          VALUES(motherId, underlyingId, newWeight);

  RETURN 1;

END
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION <schema>.underlying_insert
--
