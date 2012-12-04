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

CREATE OR REPLACE FUNCTION :filu.ftype_duplicate()
RETURNS TRIGGER AS
$BODY$
DECLARE
  mExist      int4;

BEGIN

  new.ftype_id := COALESCE(new.ftype_id, 0);

  -- Try to update
  UPDATE :filu.ftype
      SET caption    = new.caption
      WHERE ftype_id = new.ftype_id;

  IF FOUND THEN RETURN NULL; END IF;

  -- Not updated, check if caption exist
  SELECT INTO mExist :filu.id_from_caption('ftype', new.caption);

  IF mExist > 0 THEN RETURN NULL; END IF; -- Nothing todo

  RETURN new;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;

DROP TRIGGER IF EXISTS :filu_ftype_duplicates ON :filu.ftype;

CREATE TRIGGER :filu_ftype_duplicates
  BEFORE INSERT
  ON :filu.ftype
  FOR EACH ROW
  EXECUTE PROCEDURE :filu.ftype_duplicate();
