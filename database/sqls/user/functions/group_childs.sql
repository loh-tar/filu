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
CREATE OR REPLACE FUNCTION :user.group_childs
(
  aId int4
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
