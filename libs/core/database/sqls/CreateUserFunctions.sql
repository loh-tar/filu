--
--   This file is part of Filu.
--
--   Copyright (C) 2007, 2010, 2011  loh.tar@googlemail.com
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
CREATE OR REPLACE FUNCTION :user.co_duplicate()
RETURNS TRIGGER AS
$BODY$

DECLARE
  mExist  int4;

BEGIN

  IF new.co_id < 1 THEN
    -- check if already exist
    SELECT INTO mExist co_id
        FROM :user.co
        WHERE
              fi_id      = new.fi_id
          and market_id  = new.market_id
          and co_plot    = new.co_plot
          and co_date    = new.co_date
          and co_type    = new.co_type
        LIMIT 1; -- Just to be on the safe side

    IF mExist IS NULL THEN
      new.co_id := nextval(':user.co_co_id_seq');
      RETURN new;
    END IF;

  ELSE
   -- check if already exist
    SELECT INTO mExist co_id
        FROM :user.co
        WHERE co_id = new.co_id;

    IF mExist IS NULL THEN RETURN new; END IF;
  END IF;

  UPDATE :user.co
      SET
        co_plot    = new.co_plot,
        co_date    = new.co_date,
        co_type    = new.co_type,
        co_argv    = new.co_argv
      WHERE
        co_id = new.co_id;

  RETURN NULL;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;

ALTER FUNCTION :user.co_duplicate() OWNER TO :filu;

DROP TRIGGER IF EXISTS :user_co_duplicate ON :user.co;

CREATE TRIGGER :user_co_duplicate BEFORE INSERT
  ON :user.co FOR EACH ROW
  EXECUTE PROCEDURE :user.co_duplicate();
--
-- END OF FUNCTION :user.co_duplicate
--
--
CREATE OR REPLACE FUNCTION :user.group_duplicate()
RETURNS TRIGGER AS
$BODY$

BEGIN

  IF new.group_id = 0 THEN
    new.group_id := nextval(':user.group_group_id_seq');
    RETURN new;

  ELSE
    UPDATE :user.group
        SET caption        = new.caption,
            mothergroup_id = new.mothergroup_id
        WHERE group_id = new.group_id;

    IF FOUND THEN RETURN NULL; END IF;

    RETURN new;
  END IF;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;

ALTER FUNCTION :user.group_duplicate() OWNER TO :filu;

DROP TRIGGER IF EXISTS :user_group_duplicate ON :user.group;

CREATE TRIGGER :user_group_duplicate
  BEFORE INSERT
  ON :user.group
  FOR EACH ROW
  EXECUTE PROCEDURE :user.group_duplicate();
--
-- END OF FUNCTION :user.group_duplicate
--
--
CREATE OR REPLACE FUNCTION :user.group_insert
(
  aPath varchar
)
RETURNS bigint AS
$BODY$
DECLARE
  mGroupId      :user.group.group_id%TYPE;       --group id
  mMGroupId     :user.group.mothergroup_id%TYPE; --mother group id
  mGroups       text[];
  mRec          record;
  i             int;

BEGIN
  mGroups := regexp_split_to_array(aPath, '/');
  i := 0;
  mMGroupId := 0;

LOOP
  i := i + 1;

  EXIT WHEN mGroups[i] IS NULL;
  CONTINUE WHEN mGroups[i]='';

  SELECT * INTO mRec
      FROM :user.group g
      WHERE lower(g.caption) = lower(mGroups[i])
        and mothergroup_id = mMGroupId;

  mGroupId := mRec.group_id;

  IF mGroupId IS NULL THEN
    mGroupId := nextval(':user.group_group_id_seq');
    --RAISE INFO 'add new group %, g=%, m=%', mGroups[i], mGroupId, mMGroupId;
    INSERT INTO :user.group(group_id, caption, mothergroup_id)
           VALUES(mGroupId, mGroups[i], mMGroupId);
  END IF;

  mMGroupId := mGroupId;

END LOOP;

RETURN mMGroupId;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;

ALTER FUNCTION :user.group_insert(varchar) OWNER TO :filu;
--
-- END OF FUNCTION :user.group_insert
--
--
CREATE OR REPLACE FUNCTION :user.gmember_duplicate()
RETURNS TRIGGER AS
$BODY$

DECLARE
  mExist  int4;

BEGIN

  -- check if already exist
  SELECT INTO mExist gmember_id
      FROM :user.gmember
      WHERE group_id = new.group_id and fi_id = new.fi_id
      LIMIT 1; -- Just to be on the safe side

  -- if yes, don't insert
  IF mExist IS NOT NULL THEN RETURN NULL; END IF;

  RETURN new;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;
ALTER FUNCTION :user.gmember_duplicate() OWNER TO :filu;

DROP TRIGGER IF EXISTS :user_gmember_duplicate ON :user.gmember;

CREATE TRIGGER :user_gmember_duplicate
  BEFORE INSERT
  ON :user.gmember
  FOR EACH ROW
  EXECUTE PROCEDURE :user.gmember_duplicate();
--
-- END OF FUNCTION :user.gmember_duplicate
--
--
CREATE OR REPLACE FUNCTION :user.group_delete()
RETURNS TRIGGER AS
$BODY$

BEGIN

  DELETE FROM :user.gmember
      WHERE group_id = old.group_id;

  DELETE FROM :user.group
      WHERE mothergroup_id = old.group_id;

  RETURN old;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;

ALTER FUNCTION :user.group_delete() OWNER TO :filu;

DROP TRIGGER IF EXISTS :user_group_delete ON :user.group;

CREATE TRIGGER :user_group_delete
  BEFORE DELETE
  ON :user.group
  FOR EACH ROW
  EXECUTE PROCEDURE :user.group_delete();
--
-- END OF FUNCTION :user.group_delete
--
--
CREATE OR REPLACE FUNCTION :user.group_id_to_path
(
  aId :user.group.group_id%TYPE
)
RETURNS varchar AS
$BODY$
DECLARE
  mMGroupId     :user.group.group_id%TYPE; --mother group id
  mPath         varchar;
  mRec          record;

BEGIN
  mPath := ''; -- clear the mPath
  mMGroupId := aId;

  WHILE (mMGroupId > 0)
  LOOP
    SELECT * INTO mRec
        FROM :user.group g WHERE g.group_id = mMGroupId;

    mPath := '/' || mRec.caption || mPath;
    mMGroupId := mRec.mothergroup_id;
    --RAISE INFO 'mid:% path:% mMGroupId:%', mMGroupId, mPath;
  END LOOP;

RETURN mPath;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;

ALTER FUNCTION :user.group_id_to_path(:user.group.group_id%TYPE) OWNER TO :filu;
--
-- END OF FUNCTION :user.group_id_to_path
--
--
CREATE OR REPLACE FUNCTION :user.group_childs
(
  aId bigint
)
RETURNS SETOF :user.group AS
$BODY$
DECLARE
    mRec :user.group%rowtype;

BEGIN

  FOR mRec IN SELECT group_id, caption, mothergroup_id
                  FROM :user.group
                  WHERE mothergroup_id = aId
                  ORDER BY group_id
  LOOP
    RETURN NEXT mRec;
    RETURN QUERY SELECT * FROM :user.group_childs(mRec.group_id);
  END LOOP;

  RETURN;

END
$BODY$
LANGUAGE PLPGSQL;
--
-- END OF FUNCTION :user.group_childs
--
--
INSERT INTO :filu.error(caption, etext) VALUES('DepotIdNF', 'DepotId not found.');

CREATE OR REPLACE FUNCTION :user.depot_insert
(
  aDepotId    :user.depot.depot_id%TYPE,-- could be 0/NULL
  aCaption    :user.depot.caption%TYPE,
  aTrader     :user.depot.trader%TYPE,
  aOwner      :user.depot.owner%TYPE,
  aCurrency   :filu.symbol.caption%TYPE,
  aBroker     :filu.broker.caption%TYPE
)
RETURNS :user.depot.depot_id%TYPE AS
$BODY$

DECLARE
  mId         :user.depot.depot_id%TYPE; -- New ID
  mCurrencyId :user.depot.currency%TYPE;
  mBrokerId   :user.depot.broker_id%TYPE;

BEGIN

  mCurrencyId := :filu.fiid_from_symbolcaption(aCurrency);
  IF mCurrencyId < 1 THEN RETURN mCurrencyId; END IF;

  mBrokerId   := :filu.id_from_caption('broker', aBroker);
  IF mBrokerId < 1 THEN RETURN mBrokerId; END IF;

  mId := COALESCE(aDepotId, 0);

  IF mId = 0 THEN
    SELECT depot_id INTO mId
        FROM :user.depot
        WHERE lower(caption) = lower(aCaption) AND lower(owner) = lower(aOwner);
  END IF;

  mId := COALESCE(mId, 0);

  IF mId = 0 THEN
    BEGIN
      mId := nextval(':user.depot_depot_id_seq');
      INSERT INTO :user.depot(depot_id, caption, trader, owner, currency, broker_id)
             VALUES(mId, aCaption, aTrader, aOwner, mCurrencyId, mBrokerId);

      RETURN mId;
      EXCEPTION WHEN foreign_key_violation THEN RETURN :filu.error_code('ForeignKV');
    END;
  END IF;

  UPDATE :user.depot
      SET caption   = aCaption,
          trader    = aTrader,
          owner     = aOwner,
          currency  = mCurrencyId,
          broker_id = mBrokerId
      WHERE depot_id = mId;

  IF FOUND THEN RETURN mId; END IF;

  RETURN :filu.error_code('DepotIdNF');

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION :user.depot_insert
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
INSERT INTO :filu.error(caption, etext) VALUES('AccountIdNF', 'AccountId not found.');

CREATE OR REPLACE FUNCTION :user.account_insert
(
  aAccountId  :user.account.account_id%TYPE,-- could be 0/NULL
  aDepotId    :user.depot.depot_id%TYPE,
  aDate       :user.account.bdate%TYPE,
  aType       :user.account.btype%TYPE,
  aText       :user.account.btext%TYPE,
  aValue      :user.account.bvalue%TYPE
)
RETURNS :user.account.account_id%TYPE AS
$BODY$

DECLARE
  mId        :user.account.account_id%TYPE; -- New ID

BEGIN

  mId := COALESCE(aAccountId, 0);

  IF mId = 0 THEN -- Check if exist
    SELECT account_id INTO mId
        FROM :user.account
        WHERE depot_id = aDepotId
          and bdate = aDate
          and btype = aType
          and btext = aText
          and bvalue = aValue;

    IF FOUND THEN RETURN mId; END IF; -- Hm, nothing todo
  END IF;

  mId := COALESCE(mId, 0);

  IF mId = 0 THEN
    BEGIN
      mId := nextval(':user.account_account_id_seq');
      INSERT  INTO :user.account(account_id, depot_id, bdate, btype, btext, bvalue)
              VALUES(mId, aDepotId, aDate, aType, aText, aValue);

      RETURN mId;
      EXCEPTION WHEN foreign_key_violation THEN RETURN :filu.error_code('ForeignKV');
    END;
  END IF;

  UPDATE :user.account
      SET depot_id = aDepotId,
          bdate    = aDate,
          btype    = aType,
          btext    = aText,
          bvalue   = aValue
      WHERE account_id = mId;

  IF FOUND THEN RETURN mId; END IF;

  RETURN :filu.error_code('AccountIdNF');

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION :user.account_insert
--
--
CREATE OR REPLACE FUNCTION :user.depotpos_traderview
(
  aDepotId   :user.depot.depot_id%TYPE,
  aFiId      :filu.fi.fi_id%TYPE
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
          WHERE depot_id = aDepotId and fi_id = rfi_id
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
