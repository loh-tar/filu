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

CREATE OR REPLACE FUNCTION <schema>.broker_insert
  (
    nbroker_id  <schema>.broker.broker_id%TYPE,-- could be 0/NULL
    ncaption    <schema>.broker.caption%TYPE,
    nfeeformula <schema>.broker.feeformula%TYPE
  )
  RETURNS <schema>.broker.broker_id%TYPE AS
$BODY$

DECLARE
  nid        <schema>.broker.broker_id%TYPE; -- New ID
  numrows    int4;

BEGIN
  -- Insert or update an broker position.
  -- Returns
  --  -1 if broker_id is unknown

  nid := COALESCE(nbroker_id, 0);

  IF nid = 0 THEN
      nid := nextval('<schema>.broker_broker_id_seq');
      INSERT  INTO <schema>.broker(broker_id, caption, feeformula)
              VALUES(nid, ncaption, nfeeformula);

      RETURN nid;

  ELSE
      UPDATE <schema>.broker
          SET caption    = ncaption,
              feeformula = nfeeformula
          WHERE broker_id = nbroker_id;

      GET DIAGNOSTICS numrows = ROW_COUNT;
      IF numrows > 0 THEN RETURN nbroker_id;
      ELSE RETURN -1;
      END IF;

  END IF;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION <schema>.broker_insert
--
