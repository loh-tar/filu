/*
 *   This file is part of Filu.
 *
 *   Copyright (C) 2007, 2010, 2011, 2012  loh.tar@googlemail.com
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

INSERT INTO :filu.error(caption, etext) VALUES('BrokerIdNF', 'Broker Id not found or quality to bad.');

CREATE OR REPLACE FUNCTION :filu.broker_insert
(
  aCaption    :filu.broker.caption%TYPE,
  aCurrSymbol :filu.symbol.caption%TYPE,
  aFeeFormula :filu.broker.feeformula%TYPE,
  aQuality    :filu.broker.quality%TYPE,
  aBrokerId   :filu.broker.broker_id%TYPE-- could be 0/NULL
)
RETURNS :filu.broker.broker_id%TYPE AS
$BODY$

DECLARE
  mId         :filu.broker.broker_id%TYPE; -- New ID
  mCurrId     :filu.fi.fi_id%TYPE;

BEGIN
-- See also split_insert for a more smart check if exist/updateable

  mCurrId := :filu.fiid_from_symbolcaption(aCurrSymbol);
  IF mCurrId < 1 THEN RETURN mCurrId; END IF;

  mId := COALESCE(aBrokerId, 0);

  IF mId = 0 THEN mId := :filu.id_from_caption('broker', aCaption); END IF;

  IF mId < 1 THEN
      mId := nextval(':filu.broker_broker_id_seq');
      INSERT INTO :filu.broker(broker_id, caption, currency_fi_id, feeformula, quality)
             VALUES(mId, aCaption, mCurrId, aFeeFormula, aQuality);

      RETURN mId;

  ELSE
      UPDATE :filu.broker
          SET caption        = aCaption,
              currency_fi_id = mCurrId,
              feeformula     = aFeeFormula,
              quality        = aQuality
          WHERE broker_id = mId and quality >= aQuality;

      IF FOUND
        THEN RETURN mId;
        ELSE RETURN :filu.error_code('BrokerIdNF');
      END IF;

  END IF;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION :filu.broker_insert
--
