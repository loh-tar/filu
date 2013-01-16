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
INSERT INTO :filu.error(caption, etext) VALUES('DepotIdNF', 'DepotId not found.');
INSERT INTO :filu.error(caption, etext) VALUES('BrokerNF', 'Broker not found.');

CREATE OR REPLACE FUNCTION :user.depot_insert
(
  aDepotId    :user.depot.depot_id%TYPE,-- could be 0/NULL
  aCaption    :user.depot.caption%TYPE,
  aTrader     :user.depot.trader%TYPE,
  aOwner      :user.depot.owner%TYPE,
  aBroker     :filu.broker.caption%TYPE
)
RETURNS :user.depot.depot_id%TYPE AS
$BODY$

DECLARE
  mId         :user.depot.depot_id%TYPE; -- New ID
  mBrokerId   :user.depot.broker_id%TYPE;

BEGIN

  mBrokerId   := :filu.id_from_caption('broker', aBroker);
  IF mBrokerId < 1 THEN RETURN :filu.error_code('BrokerNF'); END IF;

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
      INSERT INTO :user.depot(depot_id, caption, trader, owner, broker_id)
             VALUES(mId, aCaption, aTrader, aOwner, mBrokerId);

      RETURN mId;
      EXCEPTION WHEN foreign_key_violation THEN RETURN :filu.error_code('ForeignKV');
    END;
  END IF;

  UPDATE :user.depot
      SET caption   = aCaption,
          trader    = aTrader,
          owner     = aOwner,
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
