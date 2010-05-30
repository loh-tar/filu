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
    Here is how we call an fi
*/

CREATE TABLE <schema>.symbol(
  symbol_id        SERIAL8        ,
  market_id        INT8           NOT NULL,
  stype_id         INT8           NOT NULL,
  fi_id            INT8           NOT NULL,
  caption          VARCHAR(100)   NOT NULL,
  issuedate        DATE           NOT NULL DEFAULT '1000-01-01',
  maturitydate     DATE           NOT NULL DEFAULT '3000-12-31',
  PRIMARY KEY(symbol_id),
  FOREIGN KEY(fi_id) REFERENCES <schema>.fi(fi_id) ON DELETE CASCADE,
  FOREIGN KEY(market_id) REFERENCES <schema>.market(market_id) ON DELETE CASCADE,
  FOREIGN KEY(stype_id) REFERENCES <schema>.stype(stype_id) ON DELETE CASCADE,
-- It is possible that the same caption is used by different providers, and
-- theoretical is it possible that these same caption is than used for different
-- FIs in the rare case that both use own running ID numbers like www.onvista.de
  UNIQUE(caption, market_id, stype_id),
  UNIQUE(fi_id, market_id, stype_id)
);

-- Here not UNIQUE, because of above UNIQUE(caption, market_id, stype_id).
CREATE INDEX symbol_lower_caption ON <schema>.symbol(
  lower(caption)
);

CREATE OR REPLACE FUNCTION <schema>.symbol_duplicate()
  RETURNS "trigger" AS
$BODY$
  DECLARE
      allreadyhere  BIGINT;

  BEGIN

        SELECT INTO allreadyhere symbol_id
          FROM <schema>.symbol
        WHERE (LOWER(caption) = LOWER(new.caption)
                AND market_id = new.market_id
                AND stype_id = new.stype_id
              )
            OR symbol_id = new.symbol_id
        ;
        --RAISE NOTICE 'Symbol % exists? %', allreadyhere, new.caption;

      IF allreadyhere IS NOT NULL THEN new.symbol_id := allreadyhere;  END IF;

      IF allreadyhere IS NOT NULL THEN
         --RAISE NOTICE 'Symbol % allready exists! Update Symbol %', allreadyhere, new.caption;
         UPDATE <schema>.symbol
            SET market_id     = new.market_id,
                 stype_id     = new.stype_id,
                 caption      = new.caption,
                 fi_id        = new.fi_id,
                 issuedate    = new.issuedate,
                 maturitydate = new.maturitydate
           WHERE symbol_id = allreadyhere
         ;
         RETURN NULL;
      ELSE
         RETURN new;
      END IF;
   END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE
;

CREATE TRIGGER <schema>_symbol_duplicates
  BEFORE INSERT
  ON <schema>.symbol
  FOR EACH ROW
  EXECUTE PROCEDURE <schema>.symbol_duplicate()
;
