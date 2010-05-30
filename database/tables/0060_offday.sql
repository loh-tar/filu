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
    saving market holliday usable for interest calculations
*/

CREATE TABLE <schema>.offday(
  offday_id      SERIAL8           ,
  market_id      INT8              NOT NULL,
  offday         DATE              NOT NULL,
  PRIMARY KEY(offday_id),
  FOREIGN KEY(market_id) REFERENCES <schema>.market(market_id) ON DELETE CASCADE,
  UNIQUE(market_id, offday)
);

CREATE OR REPLACE FUNCTION <schema>.offday_duplicate()
  RETURNS "trigger" AS
'
  DECLARE
      allreadyhere  BIGINT;

  BEGIN
      SELECT INTO allreadyhere offday_id 
        FROM <schema>.offday
       WHERE market_id = new.market_id
         AND offday    = new.offday
      ;

      IF allreadyhere IS NOT NULL THEN
         -- It makes no sense to update a duplicate date just do nothing!
         --RAISE WARNING ''Off day allready exists! Nothing happened'';
         RETURN NULL;
      ELSE
         RETURN new;
      END IF;
   END;
'
  LANGUAGE 'plpgsql' VOLATILE
;

CREATE TRIGGER <schema>_offday_duplicates
  BEFORE INSERT
  ON <schema>.offday
  FOR EACH ROW
  EXECUTE PROCEDURE <schema>.offday_duplicate()
;
