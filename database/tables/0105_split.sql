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

-- Table: <schema>.split
-- DROP TABLE <schema>.split;
CREATE TABLE <schema>.split
(
  split_id  SERIAL8   NOT NULL,
  fi_id     INT8      NOT NULL,
  sdate     DATE      NOT NULL,
  sratio    FLOAT     NOT NULL,
  scomment  VARCHAR(30),
  quality   INT2      DEFAULT 2, -- bronze, as tempo classified data
  CONSTRAINT split_pkey PRIMARY KEY (split_id),
  CONSTRAINT split_fi_id_fkey FOREIGN KEY (fi_id)
  REFERENCES <schema>.fi (fi_id) MATCH SIMPLE ON UPDATE NO ACTION ON DELETE CASCADE,
  UNIQUE(fi_id, sdate)
)
WITHOUT OIDS;
--ALTER TABLE <schema>.split OWNER TO <user>;

/* ------------------------------- TRIGGERS ---------------------------------------------*/

/*
Check if an entry allready exists.
If yes, just update do not die cause of duplicate key constraint.
*/
--
CREATE OR REPLACE FUNCTION <schema>.split_duplicate()
  RETURNS "trigger" AS
$BODY$

DECLARE
numrows  int4;
allreadyhere  BIGINT;

BEGIN
  -- try to update data
  UPDATE <schema>.split
    SET sdate     = new.sdate,
        sratio    = new.sratio,
        scomment  = new.scomment
    WHERE fi_id   = new.fi_id AND
          sdate   = new.sdate AND
          quality >= new.quality;

  -- if no update performed insert data
  GET DIAGNOSTICS numrows = ROW_COUNT;
  --RAISE NOTICE ''Count is %'', numrows;
  IF numrows > 0 THEN
    --RAISE NOTICE ''updated'';
    RETURN NULL;
  ELSE
    -- check if data exist but quality to bad
    SELECT INTO allreadyhere split_id
    FROM <schema>.split
    WHERE
      fi_id   = new.fi_id AND
      sdate   = new.sdate;

    IF allreadyhere IS NOT NULL THEN RETURN NULL; END IF;

    -- no, its not here, make an insert
    IF new.split_id IS NULL OR (new.split_id = 0) -- but only if id is not set
    THEN new.split_id := nextval('<schema>.split_split_id_seq'); END IF;
    --RAISE NOTICE ''inserted'';
    RETURN new;
  END IF;

END;
$BODY$
LANGUAGE 'plpgsql' VOLATILE;

CREATE TRIGGER <schema>_split_duplicates
  BEFORE INSERT
  ON <schema>.split
  FOR EACH ROW
  EXECUTE PROCEDURE <schema>.split_duplicate()
;

