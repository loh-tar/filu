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

CREATE OR REPLACE VIEW <schema>.lovelysymbol AS

  SELECT DISTINCT ON (s.fi_id)
    s.caption AS symbol,
    s.fi_id,
    s.symbol_id

    FROM <schema>.symbol s
    JOIN <schema>.stype st USING (stype_id)
  ORDER BY s.fi_id, st.seq;

--
-- END OF VIEW <schema>.lovelysymbol
--
