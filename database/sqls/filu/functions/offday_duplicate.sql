/*
 *   This file is part of Filu.
 *
 *   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
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

CREATE OR REPLACE FUNCTION :filu.offday_duplicate()
RETURNS TRIGGER AS
$BODY$
DECLARE
  mExist  int4;

BEGIN

  SELECT INTO mExist offday_id
      FROM :filu.offday
      WHERE market_id = new.market_id
        and offday    = new.offday;

  IF mExist IS NOT NULL THEN
      -- It makes no sense to update a duplicate date just do nothing!
      --RAISE WARNING ''Off day allready exists! Nothing happened'';
      RETURN NULL;
  ELSE
      RETURN new;
  END IF;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;

DROP TRIGGER IF EXISTS :filu_offday_duplicates ON :filu.offday;

CREATE TRIGGER :filu_offday_duplicates
  BEFORE INSERT
  ON :filu.offday
  FOR EACH ROW
  EXECUTE PROCEDURE :filu.offday_duplicate();
