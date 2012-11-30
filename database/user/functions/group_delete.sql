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

--ALTER FUNCTION :user.group_delete() OWNER TO :dbuser;

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
