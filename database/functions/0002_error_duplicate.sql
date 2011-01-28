/*
 *   This file is part of Filu.
 *
 *   Copyright (C) 2007, 2010  loh.tar@googlemail.com
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

CREATE OR REPLACE FUNCTION <schema>.error_duplicate()
RETURNS TRIGGER AS
$BODY$
DECLARE
  mExist   <schema>.error.error_id%TYPE;
  mErrEmpty CONSTANT int := <schema>.error_code('CaptionEY');

BEGIN

  SELECT INTO mExist <schema>.id_from_caption('error', new.caption);

  IF mExist > 0 THEN
      RAISE WARNING 'ErrorType "%" allready exists! Now updated to "%".', new.caption, new.etext;
      UPDATE <schema>.error
        SET caption = new.caption
          , etext = new.etext
        WHERE error_id = mExist;

      RETURN NULL;
  END IF;

  IF mExist = mErrEmpty THEN
    RAISE WARNING 'Errortype is empty, nothing happens.%', mExist;
    RETURN NULL;
  END IF;

  RAISE INFO 'New ErrorType: "%":"%"', new.caption, new.etext;
  RETURN new;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;

DROP TRIGGER IF EXISTS <schema>_error_duplicates ON <schema>.error;

CREATE TRIGGER <schema>_error_duplicates
  BEFORE INSERT
  ON <schema>.error
  FOR EACH ROW
  EXECUTE PROCEDURE <schema>.error_duplicate();
