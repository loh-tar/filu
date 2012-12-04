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

--INSERT INTO :filu.error(caption, etext) VALUES('', '.');

CREATE OR REPLACE FUNCTION :filu.split_insert
(
  aSymbol    :filu.symbol.caption%TYPE,
  aDate      :filu.split.sdate%TYPE,
  aRatio     :filu.split.sratio%Type,
  aComment   :filu.split.scomment%TYPE,
  aQuality   :filu.split.quality%TYPE,
  aFiId      :filu.fi.fi_id%TYPE DEFAULT 0
)
RETURNS int2 AS
$BODY$

DECLARE
  mFiId       :filu.fi.fi_id%TYPE;
  mSplitId    :filu.split.split_id%TYPE;
  mQuality    :filu.split.quality%TYPE;

BEGIN

  mFiId := aFiId;

  IF mFiId < 1 THEN
    mFiId := :filu.fiid_from_symbolcaption(aSymbol);
    IF mFiId < 1 THEN RETURN mFiId; END IF; -- The error code
  END IF;

  -- check if data exist
  SELECT INTO mSplitId, mQuality
              split_id, quality
    FROM :filu.split
    WHERE
      fi_id   = mFiId and
      sdate   = aDate;

  IF NOT FOUND THEN -- make the insert
    mSplitId := nextval(':filu.split_split_id_seq');
    INSERT INTO :filu.split(split_id, fi_id, sdate, sratio, scomment, quality)
           VALUES(mSplitId, mFiId, aDate, aRatio, aComment, aQuality);

    RETURN mSplitId;
  END IF;

  IF mQuality < aQuality THEN RETURN :filu.error_code('QualityToBad'); END IF;

  -- make an update
  UPDATE :filu.split
    SET sdate     = aDate,
        sratio    = aRatio,
        scomment  = aComment,
        quality   = aQuality
    WHERE split_id = mSplitId;

  RETURN mSplitId;

END
$BODY$
LANGUAGE PLPGSQL VOLATILE;
--
-- END OF FUNCTION :filu.split_insert
--
