--
--   This file is part of Filu.
--
--   Copyright (C) 2007, 2010  loh.tar@googlemail.com
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

CREATE OR REPLACE FUNCTION :user.depot_insert
  (
    ndepot_id   :user.depot.depot_id%TYPE,-- could be 0/NULL
    ncaption    :user.depot.caption%TYPE,
    ntrader     :user.depot.trader%TYPE,
    nowner      :user.depot.owner%TYPE,
    ncurrency   :user.depot.currency%TYPE,
    nbroker_id  :user.depot.broker_id%TYPE
  )
  RETURNS :user.depot.depot_id%TYPE AS
$BODY$

DECLARE
  nid        :user.depot.depot_id%TYPE; -- New ID
  numrows    int4;

BEGIN
  -- Insert or update an depot position.
  -- Returns
  --  -1 if depot_id is unknown
  --  -2 if depot_id or market_id is unknown

  nid := COALESCE(ndepot_id, 0);

  IF nid = 0 THEN
      nid := nextval(':user.depot_depot_id_seq');
      INSERT  INTO :user.depot(depot_id, caption, trader, owner, currency, broker_id)
              VALUES(nid, ncaption, ntrader, nowner, ncurrency, nbroker_id);

      RETURN nid;

  ELSE
      UPDATE :user.depot
          SET caption   = ncaption,
              trader    = ntrader,
              owner     = nowner,
              currency  = ncurrency,
              broker_id = nbroker_id
          WHERE depot_id = ndepot_id;

      GET DIAGNOSTICS numrows = ROW_COUNT;
      IF numrows > 0 THEN RETURN ndepot_id;
      ELSE RETURN -1;
      END IF;

  END IF;

  EXCEPTION WHEN foreign_key_violation THEN RETURN -2;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION :user.depot_insert
--

CREATE OR REPLACE FUNCTION :user.depotpos_insert
  (
    ndepotpos_id :user.depotpos.depotpos_id%TYPE,-- could be 0/NULL
    ndepot_id    :user.depot.depot_id%TYPE,
    npdate       :user.depotpos.pdate%TYPE,
    nfi_id       :user.depotpos.fi_id%TYPE,
    npieces      :user.depotpos.pieces%TYPE,
    nprice       :user.depotpos.price%TYPE,
    nmarket_id   :user.depotpos.market_id%TYPE
  )
  RETURNS :user.depotpos.depotpos_id%TYPE AS
$BODY$

DECLARE
  nid        :user.depotpos.depotpos_id%TYPE; -- New ID
  numrows    int4;

BEGIN
  -- Insert or update an depotpos position.
  -- Returns
  --  -1 if depotpos_id is unknown
  --  -2 if depot_id or market_id is unknown

  nid := COALESCE(ndepotpos_id, 0);

  IF nid = 0 THEN
      nid := nextval(':user.depotpos_depotpos_id_seq');
      INSERT  INTO :user.depotpos(depotpos_id, depot_id, pdate, fi_id, pieces, price, market_id)
              VALUES(nid, ndepot_id, npdate, nfi_id, npieces, nprice, nmarket_id);

      RETURN nid;

  ELSE
      UPDATE :user.depotpos
          SET depot_id = ndepot_id,
              pdate    = npdate,
              fi_id    = nfi_id,
              pieces   = npieces,
              price    = nprice,
              market_id= nmarket_id
          WHERE depotpos_id = ndepotpos_id;

      GET DIAGNOSTICS numrows = ROW_COUNT;
      IF numrows > 0 THEN RETURN ndepotpos_id;
      ELSE RETURN -1;
      END IF;

  END IF;

  EXCEPTION WHEN foreign_key_violation THEN RETURN -2;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION :user.depotpos_insert
--

CREATE OR REPLACE FUNCTION :user.order_insert
  (
    norder_id   :user.order.order_id%TYPE,-- could be 0/NULL
    ndepot_id   :user.depot.depot_id%TYPE,
    nodate      :user.order.odate%TYPE,
    nvdate      :user.order.vdate%TYPE,
    nfi_id      :user.order.fi_id%TYPE,
    npieces     :user.order.pieces%TYPE,
    nolimit     :user.order.olimit%TYPE,
    nbuy        :user.order.buy%TYPE,
    nmarket_id  :user.order.market_id%TYPE,
    nstatus     :user.order.status%TYPE
  )
  RETURNS :user.order.order_id%TYPE AS
$BODY$

DECLARE
  nid        :user.order.order_id%TYPE; -- New ID
  numrows    int4;

BEGIN
  -- Insert or update an order position.
  -- Returns
  --  -1 if order_id is unknown
  --  -2 if depot_id or market_id is unknown

  nid := COALESCE(norder_id, 0);

  IF nid = 0 THEN
      nid := nextval(':user.order_order_id_seq');
      INSERT  INTO :user.order(order_id, depot_id, odate, vdate, fi_id
                             , pieces, olimit, buy, market_id, status)

              VALUES(nid, ndepot_id, nodate, nvdate, nfi_id
                   , npieces, nolimit, nbuy, nmarket_id, nstatus);

      RETURN nid;

  ELSE
      UPDATE :user.order
          SET depot_id = ndepot_id,
              odate    = nodate,
              vdate    = nvdate,
              fi_id    = nfi_id,
              pieces   = npieces,
              olimit   = nolimit,
              buy      = nbuy,
              market_id= nmarket_id,
              status   = nstatus
          WHERE order_id = norder_id;

      GET DIAGNOSTICS numrows = ROW_COUNT;
      IF numrows > 0 THEN RETURN norder_id;
      ELSE RETURN -1;
      END IF;

  END IF;

  EXCEPTION WHEN foreign_key_violation THEN RETURN -2;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION :user.order_insert
--

CREATE OR REPLACE FUNCTION :user.account_insert
  (
    naccount_id :user.account.account_id%TYPE,-- could be 0/NULL
    ndepot_id   :user.depot.depot_id%TYPE,
    ndate       :user.account.bdate%TYPE,
    ntype       :user.account.btype%TYPE,
    ntext       :user.account.btext%TYPE,
    nvalue      :user.account.bvalue%TYPE
  )
  RETURNS :user.account.account_id%TYPE AS
$BODY$

DECLARE
  nid        :user.account.account_id%TYPE; -- New ID
  numrows    int4;

BEGIN
  -- Insert or update an account position.
  -- Returns
  --  -1 if account_id is unknown
  --  -2 if depot_id is unknown

  nid := COALESCE(naccount_id, 0);

  IF nid = 0 THEN
      nid := nextval(':user.account_account_id_seq');
      INSERT  INTO :user.account(account_id, depot_id, bdate, btype, btext, bvalue)
              VALUES(nid, ndepot_id, ndate, ntype, ntext, nvalue);

      RETURN nid;

  ELSE
      UPDATE :user.account
          SET depot_id = ndepot_id,
              bdate    = ndate,
              btype    = ntype,
              btext    = ntext,
              bvalue   = nvalue
          WHERE account_id = naccount_id;

      GET DIAGNOSTICS numrows = ROW_COUNT;
      IF numrows > 0 THEN RETURN naccount_id;
      ELSE RETURN -1;
      END IF;

  END IF;

  EXCEPTION WHEN foreign_key_violation THEN RETURN -2;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION :user.account_insert
--

CREATE OR REPLACE FUNCTION :user.depotpos_traderview
  (
    depotId   :user.depot.depot_id%TYPE,
    fiId      :filu.fi.fi_id%TYPE
  )
  RETURNS TABLE(rpdate     :user.depotpos.pdate%TYPE
              , rfi_id     :user.depotpos.fi_id%TYPE
              , rfiname    :filu.fi.caption%TYPE
              , rpieces    :user.depotpos.pieces%TYPE
              , rprice     :user.depotpos.price%TYPE
              , rmarket_id :user.depotpos.market_id%TYPE ) AS
$BODY$

DECLARE
  mpieces   rpieces%TYPE;
  mprice    rprice%TYPE;
  sumPieces rpieces%TYPE;
  sumPrice  rprice%TYPE;

BEGIN

  FOR rpdate, rfi_id, rfiname, rpieces, rmarket_id IN
    SELECT max(pdate), fi_id, f.caption, sum(pieces), m.currency_fi_id, market_id
      FROM :user.depotpos AS p
      LEFT JOIN :filu.fi AS f USING(fi_id)
      LEFT JOIN :filu.market AS m USING(market_id)
      WHERE p.depot_id = depotId
            and CASE WHEN fiId = -1  THEN true ELSE p.fi_id = fiId END
      GROUP BY fi_id, f.caption, m.currency_fi_id, market_id
      --ORDER BY max(pdate) DESC
  LOOP

    IF rpieces != 0
    THEN
      sumPieces :=  0;
      sumPrice  :=  0.0;
      rprice    := -1.0; -- Mark as 'not ready calculated'

      FOR mpieces, mprice IN
        SELECT pieces, price
          FROM :user.depotpos
          WHERE depot_id = depotId and fi_id = rfi_id
          ORDER BY pdate DESC
      LOOP
        IF mpieces < 0
        THEN -- Ah, was sell use this as entry price
          IF sumPieces = 0
            THEN rprice := mprice;
            ELSE rprice := (mprice + (sumPrice / sumPieces)) / 2;
          END IF;
          EXIT;
        END IF;

        sumPieces := sumPieces + mpieces;
        IF sumPieces > rpieces
        THEN
          rprice := sumPrice / rpieces;
          EXIT;
        END IF;

        sumPrice := sumPrice + (mpieces * mprice);

      END LOOP;

      IF rprice < 0 THEN rprice := sumPrice / rpieces; END IF;
      RETURN NEXT;

    END IF;

  END LOOP;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION :user.depotpos_traderview
--
