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


-- Function: <schema>.eodbar_get(int8, int8, date, date, int8, int8)

-- DROP FUNCTION <schema>.eodbar_get(int8, int8, date, date, int8, int8);

CREATE OR REPLACE FUNCTION <schema>.eodbar_get(int8, int8, date, date, int8,
int8)
  RETURNS SETOF <schema>.fbar AS
$BODY$
DECLARE
  query        TEXT;
  generic_rec  RECORD;
  my_fi_id     ALIAS FOR $1;
  my_market_id ALIAS FOR $2;
  my_start_dt  ALIAS FOR $3;
  my_end_dt    ALIAS FOR $4;
  my_limit     ALIAS FOR $5;  -- max number of rows
  my_order     ALIAS FOR $6;  -- sorting: -1 desc / 1 asc / 0 random
  fdate        DATE;
  tdate        DATE;
  my_result    <schema>.fbar;
  indicator    INT8;
  splitratio   float4;
  splitrecord  RECORD;
  splitdate    DATE;

BEGIN
  SELECT INTO indicator f.indicator_id
    FROM <schema>.fi f, <schema>.ftype t
   WHERE f.fi_id = my_fi_id
     AND f.ftype_id = t.ftype_id
     AND LOWER(t.caption) = 'generic';

  IF my_start_dt IS NULL THEN
      fdate := '1900-01-01';
  ELSE
      fdate := my_start_dt;
  END IF;

  IF my_end_dt IS NULL THEN
      tdate := '3000-12-31';
  ELSE
      tdate := my_end_dt;
  END IF;

  -- fetch last date if limited rowcount needed
  IF my_limit IS NOT NULL THEN
      SELECT INTO fdate qdate FROM <schema>.eodbar
       WHERE fi_id = my_fi_id
         AND market_id = my_market_id
         AND qdate <= tdate
       ORDER BY qdate DESC
       LIMIT my_limit offset my_limit-1;
  END IF;

  -- if fdate is still null override it
  IF fdate IS NULL THEN
      fdate := '1000-01-01';
  END IF;

  -- if not generic fi define normal sql
  IF indicator IS NULL THEN

    query := 'SELECT *,
                COALESCE((select exp(sum(ln(sratio)))
                            from <schema>.split s
                            where s.sdate > e.qdate and s.fi_id = e.fi_id),1)
                            as sratio

              FROM <schema>.eodbar e
              WHERE
                e.fi_id = <fi_id>
                AND qdate BETWEEN ''<fdate>'' AND ''<tdate>''
                AND market_id = <market_id> ';

  ELSE
      SELECT INTO query sql FROM <schema>.indicator
       WHERE indicator_id = indicator;
  END IF;

  -- add order by
  IF my_order > 0 THEN -- asc
      query := query || 'ORDER BY qdate ASC';
  ELSEIF my_order < 0 THEN --desc
      query := query || 'ORDER BY qdate DESC';
  ELSE -- random/Scramble
      query := query || 'ORDER BY random()';
  END IF;

  query := REPLACE(query, '<fi_id>',     CAST(my_fi_id AS text) );
  query := REPLACE(query, '<fdate>',     CAST(fdate AS text) );
  query := REPLACE(query, '<tdate>',     CAST(tdate AS text) );
  query := REPLACE(query, '<market_id>', CAST(my_market_id AS text) );

  --RAISE NOTICE ' query:= % ', query;

  -- adjust data to splits
  FOR generic_rec IN EXECUTE query
  LOOP
    my_result.fdate  := generic_rec.qdate;
    my_result.ftime  := '23:59:59'::Time;

    my_result.fopen  := generic_rec.qopen  * generic_rec.sratio;
    my_result.fhigh  := generic_rec.qhigh  * generic_rec.sratio;
    my_result.flow   := generic_rec.qlow   * generic_rec.sratio;
    my_result.fclose := generic_rec.qclose * generic_rec.sratio;
    my_result.fvol   := generic_rec.qvol ;--  / generic_rec.sratio;

    my_result.foi    := generic_rec.qoi;

    RETURN NEXT my_result;
  END LOOP;

  RETURN;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
-- ALTER FUNCTION <schema>.eodbar_get(int8, int8, date, date, int8, int8)
-- OWNER TO <user>;
--
-- END OF FUNCTION <schema>.eodbar_get(...)
--
