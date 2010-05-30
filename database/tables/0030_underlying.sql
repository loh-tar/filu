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
/*
    FiMi

    Copyright (C) 2001-2006 Christian Kindler

    This file is part of FiMi.

    FiMi is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*
    Here we store all underlyings an fi can have
*/

CREATE TABLE <schema>.underlying(
  underlying_id    SERIAL8        ,
  fi_id            INT8           NOT NULL,            --must not be serial cause n:n relation to fi.fi
  weight           FLOAT          NOT NULL DEFAULT 1,
  underlying_fi_id INT8           NOT NULL,
  PRIMARY KEY(underlying_id),
  FOREIGN KEY(underlying_fi_id) REFERENCES <schema>.fi(fi_id) ON DELETE CASCADE,
  FOREIGN KEY(fi_id) REFERENCES <schema>.fi(fi_id) ON DELETE CASCADE,
  UNIQUE(underlying_fi_id, fi_id)
);

CREATE OR REPLACE FUNCTION <schema>.underlying_duplicate()
  RETURNS "trigger" AS
'
  DECLARE
      allreadyhere  BIGINT;

  BEGIN
      SELECT INTO allreadyhere underlying_id
        FROM <schema>.underlying
       WHERE underlying_fi_id = new.underlying_fi_id
         AND fi_id = new.fi_id
      ;

      IF allreadyhere IS NOT NULL THEN
         --RAISE WARNING ''Underlyings allready exists! Update Values'';
         UPDATE <schema>.underlying 
            SET fi_id            = new.fi_id,
                weight           = new.weight,
                underlying_fi_id = new.underlying_fi_id
            WHERE underlying_id = allreadyhere
         ;
         RETURN NULL;
      ELSE
         RETURN new;
      END IF;
   END;
'
  LANGUAGE 'plpgsql' VOLATILE
;

CREATE TRIGGER <schema>_underlying_duplicates
  BEFORE INSERT
  ON <schema>.underlying
  FOR EACH ROW
  EXECUTE PROCEDURE <schema>.underlying_duplicate()
;
