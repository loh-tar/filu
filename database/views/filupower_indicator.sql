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
--
--
-- The ultimate power and reason to start the FiMi project by Christian Kindler
--
CREATE OR REPLACE VIEW <schema>.filupower_indicator
AS
  SELECT
      proc.oid as indicator_id
    , proc.proname as caption
    , proc.proargnames as arguments
    , rt.typname as rettype
    , 'SELECT * FROM <schema>.' || proc.proname || '(:' ||array_to_string(proc.proargnames, ', :') || ')' as call
    , proc.prosrc as source

  FROM
    pg_namespace ns, pg_proc proc, pg_type rt
  WHERE ns.oid = proc.pronamespace
    AND proc.prorettype = rt.oid
    AND ns.nspname = '<schema>'
    AND proc.proname like 'fpi_%';

--
-- END OF VIEW <schema>.filupower_indicator
--
