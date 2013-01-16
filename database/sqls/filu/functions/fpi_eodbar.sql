/*
 *   This file is part of Filu.
 *
 *   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
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

CREATE OR REPLACE FUNCTION :filu.fpi_eodBar
(
    "fiId"      int4
  , "marketId"  int4
  , "fdate"     date
  , "tdate"     date
)
RETURNS SETOF :filu.fbar AS
$BODY$
DECLARE
  my_record    record;
  my_result    :filu.fbar;

BEGIN

  FOR my_record IN
      SELECT * FROM :filu.eodbar
       WHERE fi_id     = "fiId"
         and market_id = "marketId"
         and qdate BETWEEN "fdate" and "tdate"
       ORDER BY qdate ASC
  LOOP
     my_result.fdate  := my_record.qdate;
     my_result.ftime  := '23:59:59'::Time;
     my_result.fopen  := my_record.qopen;
     my_result.fhigh  := my_record.qhigh;
     my_result.flow   := my_record.qlow;
     my_result.fclose := my_record.qclose;
     my_result.fvol   := my_record.qvol;
     my_result.foi    := my_record.qoi;

     RETURN NEXT my_result;
  END LOOP;

  RETURN;
END;
$BODY$
LANGUAGE PLPGSQL VOLATILE;

--
-- END OF FUNCTION :filu.fpi_eodBar(...)
--
