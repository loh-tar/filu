--
--   This file is part of Filu.
--
--   Copyright (C) 2007, 2010, 2011, 2012  loh.tar@googlemail.com
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

--ALTER FUNCTION :user.co_duplicate() OWNER TO :dbuser;

DROP TRIGGER IF EXISTS :user_co_duplicate ON :user.co;

CREATE TRIGGER :user_co_duplicate BEFORE INSERT
  ON :user.co FOR EACH ROW
  EXECUTE PROCEDURE :user.co_duplicate();
--
-- END OF FUNCTION :user.co_duplicate
--
--
