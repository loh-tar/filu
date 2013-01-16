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

INSERT INTO :filu.error(caption, etext) VALUES('CaptionEY', 'Caption is empty.');
INSERT INTO :filu.error(caption, etext) VALUES('CaptionNF', 'Caption not found.');
INSERT INTO :filu.error(caption, etext) VALUES('CaptionNUQ', 'Caption is not unique.');

CREATE OR REPLACE FUNCTION :filu.id_from_caption
(
  aTable   varchar, -- search in this table
  aCaption varchar  -- search this caption
)
RETURNS int4 AS
$BODY$

DECLARE
  mRec        record;
  mId         int4;
  mCaption    varchar;
  mQuery      text;
  mCount      int4;

BEGIN
  --
  -- Returns Unique Id if found or < 0 if not or trouble
  --
  IF aCaption IS NULL THEN RETURN :filu.error_code('CaptionEY'); END IF;

  mCaption := trim(both from aCaption);
  IF char_length(mCaption) = 0 THEN RETURN :filu.error_code('CaptionEY'); END IF;

  -- go for caption
  mQuery := 'SELECT ' || quote_ident(aTable) || '_id::int4 AS id' ||
            ' FROM :filu.' || quote_ident(aTable) ||
            ' WHERE lower(caption) LIKE lower(' || quote_literal(mCaption) || ')';

  --RAISE NOTICE ':filu.id_from_caption: %', mQuery;
  mCount := 0;
  FOR mRec IN EXECUTE mQuery LOOP
      mCount := mCount +1;
      mId := mRec.id;
  END LOOP;

  IF mId IS NULL THEN mId := :filu.error_code('CaptionNF'); END IF;

  IF mCount > 1 THEN
    --RAISE NOTICE ':filu.id_from_caption: More than one caption >%< found. ', aCaption;
    mId := :filu.error_code('CaptionNUQ');
  END IF;

  RETURN mId;

END
$BODY$
LANGUAGE PLPGSQL STABLE;
--
-- END OF FUNCTION :filu.id_from_caption
--
