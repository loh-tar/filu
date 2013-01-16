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
