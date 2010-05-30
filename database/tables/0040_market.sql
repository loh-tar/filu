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
    Holding the market information
*/

CREATE TABLE <schema>.market(
  market_id      SERIAL8           ,
  caption        VARCHAR(100)      NOT NULL,
  currency_fi_id INT8              NULL,
--  MType_id       INT8              NOT NULL DEFAULT 1,
  opentime       TIME              NOT NULL DEFAULT '00:00:00',
  closetime      TIME              NOT NULL DEFAULT '23:59:00',
  PRIMARY KEY(market_id)           ,
  FOREIGN KEY(currency_fi_id) REFERENCES <schema>.fi(fi_id)
  --FOREIGN KEY(mtype_id) REFERENCES <schema>.mtype(mtype_id) ON DELETE CASCADE,
);

CREATE UNIQUE INDEX market_unique_caption ON <schema>.market(
  lower(caption)
);

CREATE OR REPLACE FUNCTION <schema>.market_duplicate()
  RETURNS "trigger" AS
'
  DECLARE
      allreadyhere  BIGINT;

  BEGIN
      SELECT INTO allreadyhere market_id
        FROM <schema>.market
       WHERE (LOWER(caption) = LOWER(new.caption))
          OR market_id = new.market_id
      ;

      IF allreadyhere IS NOT NULL THEN
         --RAISE WARNING ''Market allready exists! Nothing happened'';
         UPDATE <schema>.market
            SET caption = new.caption,
                currency_fi_id = new.currency_fi_id,
                --mtype_id       = new.mtype_id,
                opentime       = new.opentime,
                closetime      = new.closetime
          WHERE market_id = allreadyhere
          ;

         RETURN NULL;
      ELSE
         RETURN new;
      END IF;
   END;
'
  LANGUAGE 'plpgsql' VOLATILE
;

CREATE TRIGGER <schema>_market_duplicates
  BEFORE INSERT
  ON <schema>.market
  FOR EACH ROW
  EXECUTE PROCEDURE <schema>.market_duplicate()
;
