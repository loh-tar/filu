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

--ALTER FUNCTION :user.group_duplicate() OWNER TO :dbuser;

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
