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
--ALTER FUNCTION :user.gmember_duplicate() OWNER TO :dbuser;

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
