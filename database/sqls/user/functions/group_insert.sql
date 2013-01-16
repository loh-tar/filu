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

CREATE OR REPLACE FUNCTION :user.group_insert
(
  aPath varchar
)
RETURNS int4 AS
$BODY$
DECLARE
  mGroupId      :user.group.group_id%TYPE;       --group id
  mMGroupId     :user.group.mothergroup_id%TYPE; --mother group id
  mGroups       text[];
  mRec          record;
  i             int;

BEGIN
  mGroups := regexp_split_to_array(aPath, '/');
  i := 0;
  mMGroupId := 0;

LOOP
  i := i + 1;

  EXIT WHEN mGroups[i] IS NULL;
  CONTINUE WHEN mGroups[i]='';

  SELECT * INTO mRec
      FROM :user.group g
      WHERE lower(g.caption) = lower(mGroups[i])
        and mothergroup_id = mMGroupId;

  mGroupId := mRec.group_id;

  IF mGroupId IS NULL THEN
    mGroupId := nextval(':user.group_group_id_seq');
    --RAISE INFO 'add new group %, g=%, m=%', mGroups[i], mGroupId, mMGroupId;
    INSERT INTO :user.group(group_id, caption, mothergroup_id)
           VALUES(mGroupId, mGroups[i], mMGroupId);
  END IF;

  mMGroupId := mGroupId;

END LOOP;

RETURN mMGroupId;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;

--ALTER FUNCTION :user.group_insert(varchar) OWNER TO :dbuser;
--
-- END OF FUNCTION :user.group_insert
--
--
