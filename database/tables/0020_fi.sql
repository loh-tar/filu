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
    This is the main table containing the unique fi
*/

CREATE TABLE <schema>.fi(
  fi_id         SERIAL8        ,
  caption       VARCHAR(100)   NOT NULL,
  deletedate    DATE           DEFAULT '3000-12-31',
  ftype_id      INT8           NOT NULL,
  indicator_id  INT8           ,
  PRIMARY KEY (fi_id),
  FOREIGN KEY (ftype_id) REFERENCES <schema>.ftype(ftype_id) ON DELETE CASCADE--,
  -- FOREIGN KEY (indicator_id) REFERENCES <schema>.indicator(indicator_id) ON DELETE CASCADE,
  --UNIQUE (caption, ftype_id)
);

CREATE UNIQUE INDEX fi_unique_caption ON <schema>.fi(
  lower(caption)
);

CREATE OR REPLACE FUNCTION <schema>.fi_duplicate()
  RETURNS "trigger" AS
'
  DECLARE
      allreadyhere  BIGINT;

  BEGIN
      SELECT INTO allreadyhere fi_id
        FROM <schema>.fi
       WHERE (UPPER(caption) = UPPER(new.caption) AND ftype_id = new.ftype_id)
          OR fi_id = new.fi_id;

      IF allreadyhere IS NOT NULL THEN
         --RAISE WARNING ''fi allready exists! Fi updated'';
         UPDATE <schema>.fi SET deletedate   = new.deletedate,
                                caption      = new.caption,
                                ftype_id     = new.ftype_id,
                                indicator_id = new.indicator_id
                          WHERE fi_id = allreadyhere
         ;
         RETURN NULL;
      ELSE
         RETURN new;
      END IF;
   END;
'
  LANGUAGE 'plpgsql' VOLATILE
;

CREATE TRIGGER <schema>_fi_duplicates
  BEFORE INSERT
  ON <schema>.fi
  FOR EACH ROW
  EXECUTE PROCEDURE <schema>.fi_duplicate()
;


