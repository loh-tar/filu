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
INSERT INTO :filu.error(caption, etext) VALUES('DepotPosIdNF', 'DepotPositionId not found.');

CREATE OR REPLACE FUNCTION :user.depotpos_insert
(
  aDepotPosId  :user.depotpos.depotpos_id%TYPE,-- could be 0/NULL
  aDepotId     :user.depot.depot_id%TYPE,
  aPDate       :user.depotpos.pdate%TYPE,
  aFiId        :user.depotpos.fi_id%TYPE,
  aPieces      :user.depotpos.pieces%TYPE,
  aPrice       :user.depotpos.price%TYPE,
  aMarketId    :user.depotpos.market_id%TYPE,
  aNote        :user.depotpos.note%TYPE
)
RETURNS :user.depotpos.depotpos_id%TYPE AS
$BODY$

DECLARE
  mId        :user.depotpos.depotpos_id%TYPE; -- New ID

BEGIN

  mId := COALESCE(aDepotPosId, 0);

  IF mId = 0 THEN -- Check if exist
    SELECT depotpos_id INTO mId
        FROM :user.depotpos
        WHERE depot_id = aDepotId
          and pdate = aPDate
          and fi_id = aFiId
          and pieces = aPieces
          and price = aPrice
          and market_id = aMarketId;
  END IF;

  mId := COALESCE(mId, 0);

  IF mId = 0 THEN
    BEGIN
      mId := nextval(':user.depotpos_depotpos_id_seq');
      INSERT  INTO :user.depotpos(depotpos_id, depot_id, pdate, fi_id, pieces, price, market_id, note)
              VALUES(mId, aDepotId, aPDate, aFiId, aPieces, aPrice, aMarketId, aNote);

      RETURN mId;
      EXCEPTION WHEN foreign_key_violation THEN RETURN :filu.error_code('ForeignKV');
    END;
  END IF;

  UPDATE :user.depotpos
      SET depot_id = aDepotId,
          pdate    = aPDate,
          fi_id    = aFiId,
          pieces   = aPieces,
          price    = aPrice,
          market_id= aMarketId,
          note     = aNote
      WHERE depotpos_id = mId;

  IF FOUND THEN RETURN mId; END IF;

  RETURN :filu.error_code('DepotPosIdNF');

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION :user.depotpos_insert
--
--
