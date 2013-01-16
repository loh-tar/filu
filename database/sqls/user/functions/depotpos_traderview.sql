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
CREATE OR REPLACE FUNCTION :user.depotpos_traderview
(
  aDepotId   :user.depot.depot_id%TYPE,
  aFiId      :filu.fi.fi_id%TYPE,
  aDate      date
)
RETURNS TABLE(rpdate     :user.depotpos.pdate%TYPE
            , rfi_id     :user.depotpos.fi_id%TYPE
            , rfiname    :filu.fi.caption%TYPE
            , rpieces    :user.depotpos.pieces%TYPE
            , rprice     :user.depotpos.price%TYPE
            , rmarket_id :user.depotpos.market_id%TYPE ) AS
$BODY$

DECLARE
  mPieces       rpieces%TYPE;
  mPrice        rprice%TYPE;
  mSumPieces    rpieces%TYPE;
  mSumPrice     rprice%TYPE;

BEGIN

  FOR rpdate, rfi_id, rfiname, rpieces, rmarket_id IN
    SELECT max(pdate), fi_id, f.caption, sum(pieces), market_id
      FROM :user.depotpos AS p
      LEFT JOIN :filu.fi AS f USING(fi_id)
      LEFT JOIN :filu.market AS m USING(market_id)
      WHERE p.depot_id = aDepotId
            and p.pdate <= aDate
            and CASE WHEN aFiId = -1  THEN true ELSE p.fi_id = aFiId END
      GROUP BY fi_id, f.caption, market_id
      --ORDER BY max(pdate) DESC
  LOOP

    IF rpieces != 0
    THEN
      mSumPieces :=  0;
      mSumPrice  :=  0.0;
      rprice     := -1.0; -- Mark as 'not ready calculated'

      FOR mPieces, mPrice IN
        SELECT pieces, price
          FROM :user.depotpos
          WHERE depot_id = aDepotId and fi_id = rfi_id and pdate <= aDate
          ORDER BY pdate DESC
      LOOP
        IF mPieces < 0
        THEN -- Ah, was sell use this as entry price
          IF mSumPieces = 0
            THEN rprice := mPrice;
            ELSE rprice := (mPrice + (mSumPrice / mSumPieces)) / 2;
          END IF;
          EXIT;
        END IF;

        mSumPieces := mSumPieces + mPieces;
        IF mSumPieces > rpieces
        THEN
          rprice := mSumPrice / rpieces;
          EXIT;
        END IF;

        mSumPrice := mSumPrice + (mPieces * mPrice);

      END LOOP;

      IF rprice < 0 THEN rprice := mSumPrice / rpieces; END IF;
      RETURN NEXT;

    END IF;

  END LOOP;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION :user.depotpos_traderview
--
