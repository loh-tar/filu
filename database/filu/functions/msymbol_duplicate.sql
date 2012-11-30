/*
 *   This file is part of Filu.
 *
 *   Copyright (C) 2007, 2010, 2011, 2012  loh.tar@googlemail.com
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

CREATE OR REPLACE FUNCTION :schema.msymbol_duplicate()
RETURNS TRIGGER AS
$BODY$
DECLARE
  mExist  int4;

BEGIN

  SELECT INTO mExist msymbol_id
      FROM :schema.msymbol
      WHERE ( lower(caption) = lower(new.caption)
        and market_id = new.market_id
        and stype_id = new.stype_id ) or msymbol_id = new.msymbol_id;

  IF mExist IS NOT NULL THEN
    --RAISE WARNING 'Market Symbol allready exists! Nothing happened';
    RETURN NULL;
  ELSE
      RETURN new;
  END IF;

END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;

DROP TRIGGER IF EXISTS :schema_msymbol_duplicates ON :schema.msymbol;

CREATE TRIGGER :schema_msymbol_duplicates
  BEFORE INSERT
  ON :schema.msymbol
  FOR EACH ROW
  EXECUTE PROCEDURE :schema.msymbol_duplicate();
