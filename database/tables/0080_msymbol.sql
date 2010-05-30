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
    Here is how providers call a market
*/

CREATE TABLE <schema>.msymbol(
  msymbol_id       SERIAL      NOT NULL,
  stype_id         INT         NOT NULL,
  market_id        INT         NOT NULL,
  caption          varchar(20) NOT NULL,
  PRIMARY KEY(msymbol_id),
  FOREIGN KEY(stype_id) REFERENCES <schema>.stype(stype_id),
  FOREIGN KEY(market_id) REFERENCES <schema>.market(market_id)
);

CREATE OR REPLACE FUNCTION <schema>.msymbol_duplicate()
  RETURNS "trigger" AS
'
  DECLARE
      allreadyhere  BIGINT;

  BEGIN
      SELECT INTO allreadyhere msymbol_id
        FROM <schema>.msymbol
       WHERE caption = new.caption
         AND market_id = new.market_id
         AND stype_id = new.stype_id
      ;

      IF allreadyhere IS NOT NULL THEN
         --RAISE WARNING ''Market Symbol allready exists! Nothing happened'';
         RETURN NULL;
      ELSE
         RETURN new;
      END IF;
   END;
'
  LANGUAGE 'plpgsql' VOLATILE
;

CREATE TRIGGER <schema>_msymbol_duplicates
  BEFORE INSERT
  ON <schema>.msymbol
  FOR EACH ROW
  EXECUTE PROCEDURE <schema>.msymbol_duplicate()
;
