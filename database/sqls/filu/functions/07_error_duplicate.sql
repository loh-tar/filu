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

CREATE OR REPLACE FUNCTION :filu.error_duplicate()
RETURNS TRIGGER AS
$BODY$
DECLARE
  mExist   :filu.error.error_id%TYPE;
  mErrEmpty CONSTANT int := :filu.error_code('CaptionEY');

BEGIN

  SELECT INTO mExist :filu.id_from_caption('error', new.caption);

  IF mExist > 0 THEN
      RAISE NOTICE 'ErrorType "%" allready exists! Now updated to "%".', new.caption, new.etext;
      UPDATE :filu.error
        SET caption = new.caption
          , etext = new.etext
        WHERE error_id = mExist;

      RETURN NULL;
  END IF;

  IF mExist = mErrEmpty THEN
    RAISE WARNING 'Errortype is empty, nothing happens.%', mExist;
    RETURN NULL;
  END IF;

  --RAISE INFO 'New ErrorType: "%":"%"', new.caption, new.etext;
  RETURN new;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;

DROP TRIGGER IF EXISTS :filu_error_duplicates ON :filu.error;

CREATE TRIGGER :filu_error_duplicates
  BEFORE INSERT
  ON :filu.error
  FOR EACH ROW
  EXECUTE PROCEDURE :filu.error_duplicate();
