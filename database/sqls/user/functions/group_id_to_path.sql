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
CREATE OR REPLACE FUNCTION :user.group_id_to_path
(
  aId :user.group.group_id%TYPE
)
RETURNS varchar AS
$BODY$
DECLARE
  mMGroupId     :user.group.group_id%TYPE; --mother group id
  mPath         varchar;
  mRec          record;

BEGIN
  mPath := ''; -- clear the mPath
  mMGroupId := aId;

  WHILE (mMGroupId > 0)
  LOOP
    SELECT * INTO mRec
        FROM :user.group g WHERE g.group_id = mMGroupId;

    mPath := '/' || mRec.caption || mPath;
    mMGroupId := mRec.mothergroup_id;
    --RAISE INFO 'mid:% path:% mMGroupId:%', mMGroupId, mPath;
  END LOOP;

RETURN mPath;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;

--ALTER FUNCTION :user.group_id_to_path(:user.group.group_id%TYPE) OWNER TO :dbuser;
--
-- END OF FUNCTION :user.group_id_to_path
--
--
