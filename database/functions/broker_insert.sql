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

INSERT INTO <schema>.error(caption, etext) VALUES('BrokerNUQ', 'Broker name already exist, give me the ID for an update.');

CREATE OR REPLACE FUNCTION <schema>.broker_insert
(
  aCaption    <schema>.broker.caption%TYPE,
  aFeeFormula <schema>.broker.feeformula%TYPE,
  aBrokerId   <schema>.broker.broker_id%TYPE-- could be 0/NULL
)
RETURNS <schema>.broker.broker_id%TYPE AS
$BODY$

DECLARE
  mId        <schema>.broker.broker_id%TYPE; -- New ID
  mNumRows    int;

BEGIN
  -- Insert or update an broker position.
  -- Returns
  --  -1 if broker_id is unknown

  mId := COALESCE(aBrokerId, 0);

  IF mId = 0 THEN
      mId := <schema>.id_from_caption('broker', aCaption);
      IF mId > 0 THEN RETURN <schema>.error_code('BrokerNUQ'); END IF; -- FIXME add

      mId := nextval('<schema>.broker_broker_id_seq');
      INSERT  INTO <schema>.broker(broker_id, caption, feeformula)
              VALUES(mId, aCaption, aFeeFormula);

      RETURN mId;

  ELSE
      UPDATE <schema>.broker
          SET caption    = aCaption,
              feeformula = aFeeFormula
          WHERE broker_id = aBrokerId;

      GET DIAGNOSTICS mNumRows = ROW_COUNT;
      IF mNumRows > 0 THEN RETURN aBrokerId;
      ELSE RETURN -1;
      END IF;

  END IF;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION <schema>.broker_insert
--
