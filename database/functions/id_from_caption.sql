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

CREATE OR REPLACE FUNCTION <schema>.id_from_caption
(
  stable varchar, -- search in this table
  scap varchar    -- search this caption
)
  RETURNS int8 AS
$BODY$

DECLARE
    my_rec      record;
    my_id       int8;
    my_cap      varchar;
    query       text;
    cnt         int8;

BEGIN
  -- Returns:
  --    0, if no data available(caption not found)
  --   >0, found id to caption
  --   -1, caption is more than one times in table
  --   -2, caption was empty

  my_cap := trim(both from scap);
  IF char_length(my_cap) = 0 THEN RETURN -2; END IF;

  -- go for caption
  query := $$ SELECT $$ || stable || $$_id::int8 AS id FROM <schema>.$$ || stable ||
          $$ WHERE LOWER(caption) LIKE LOWER($$ || quote_literal(my_cap) || $$)$$;

  --RAISE NOTICE '<schema>.id_from_caption: %', query;
  cnt := 0;
  FOR my_rec IN EXECUTE query LOOP
      cnt := cnt +1;
      my_id := my_rec.id;
  END LOOP;

  IF my_id IS NULL THEN
      my_id := 0;
  END IF;

  IF cnt > 1 THEN
    --RAISE NOTICE '<schema>.id_from_caption: More than one caption >%< found. ', scap;
    my_id := -1;
  END IF;

  RETURN my_id;

END
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--
-- END OF FUNCTION <schema>.id_from_caption
--
