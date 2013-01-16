--
--   This file is part of Filu.
--
--   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
--
--   Filu is free software: you can redistribute it and/or modify
--   it under the terms of the GNU General Public License as published by
--   the Free Software Foundation, either version 2 of the License, or
--   (at your option) any later version.
--
--   Filu is distributed in the hope that it will be useful,
--   but WITHOUT ANY WARRANTY; without even the implied warranty of
--   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--   GNU General Public License for more details.
--
--   You should have received a copy of the GNU General Public License
--   along with Filu. If not, see <http://www.gnu.org/licenses/>.
--
--
INSERT INTO :filu.error(caption, etext) VALUES('OrderIdNF', 'OrderId not found.');

CREATE OR REPLACE FUNCTION :user.order_insert
(
  aOrderId    :user.order.order_id%TYPE,-- could be 0/NULL
  aDepotId    :user.depot.depot_id%TYPE,
  aODate      :user.order.odate%TYPE,
  aVDate      :user.order.vdate%TYPE,
  aFiId       :user.order.fi_id%TYPE,
  aPieces     :user.order.pieces%TYPE,
  aOLimit     :user.order.olimit%TYPE,
  aBuy        :user.order.buy%TYPE,
  aMarketId   :user.order.market_id%TYPE,
  aStatus     :user.order.status%TYPE,
  aNote       :user.order.note%TYPE
)
RETURNS :user.order.order_id%TYPE AS
$BODY$

DECLARE
  mId        :user.order.order_id%TYPE; -- New ID

BEGIN

  mId := COALESCE(aOrderId, 0);

  IF mId = 0 THEN -- Check if exist
    SELECT order_id INTO mId
        FROM :user.order
        WHERE depot_id = aDepotId
          and odate = aODate
          and vdate = aVDate
          and fi_id = aFiId
          and pieces = aPieces
          and olimit = aOLimit
          and buy = aBuy
          and market_id = aMarketId;
  END IF;

  mId := COALESCE(mId, 0);

  IF mId = 0 THEN
    BEGIN
      mId := nextval(':user.order_order_id_seq');
      INSERT  INTO :user.order(order_id, depot_id, odate, vdate, fi_id
                             , pieces, olimit, buy, market_id, status, note)

              VALUES(mId, aDepotId, aODate, aVDate, aFiId
                   , aPieces, aOLimit, aBuy, aMarketId, aStatus, aNote);

      RETURN mId;
      EXCEPTION WHEN foreign_key_violation THEN RETURN :filu.error_code('ForeignKV');
    END;
  END IF;

  UPDATE :user.order
      SET depot_id  = aDepotId,
          odate     = aODate,
          vdate     = aVDate,
          fi_id     = aFiId,
          pieces    = aPieces,
          olimit    = aOLimit,
          buy       = aBuy,
          market_id = aMarketId,
          status    = aStatus,
          note      = aNote
      WHERE order_id = mId;

  IF FOUND THEN RETURN mId; END IF;

  RETURN :filu.error_code('OrderIdNF');

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION :user.order_insert
--
--
