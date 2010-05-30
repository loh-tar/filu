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
    This is the table containig all types
*/

CREATE TABLE <schema>.ftype(
  ftype_id     SERIAL        PRIMARY KEY,
  caption      VARCHAR(100)  NOT NULL
);

CREATE UNIQUE INDEX ftype_unique_caption ON <schema>.ftype(
  lower(caption)
);

CREATE OR REPLACE FUNCTION <schema>.ftype_duplicate()
  RETURNS "trigger" AS
'
  DECLARE
      allreadyhere  BIGINT;

  BEGIN
      SELECT INTO allreadyhere ftype_id
        FROM <schema>.ftype
       WHERE (LOWER(caption) = LOWER(new.caption))
          OR ftype_id = new.ftype_id
      ;

      IF allreadyhere IS NOT NULL THEN
         --RAISE WARNING ''Type allready exists! Update Fi Type'';
         UPDATE <schema>.ftype
            SET caption = new.caption
          WHERE ftype_id = allreadyhere
         ;

         RETURN NULL;
      ELSE
         RETURN new;
      END IF;
   END;
'
  LANGUAGE 'plpgsql' VOLATILE
;

CREATE TRIGGER <schema>_type_duplicates
  BEFORE INSERT
  ON <schema>.ftype
  FOR EACH ROW
  EXECUTE PROCEDURE <schema>.ftype_duplicate()
;
