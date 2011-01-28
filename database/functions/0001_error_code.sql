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

INSERT INTO <schema>.error(caption, etext) VALUES('ErrorNF', 'Error key not found, should never happens!');
-- INSERT INTO <schema>.error(caption, etext) VALUES('', '.');
-- <schema>.error_code('');

CREATE OR REPLACE FUNCTION <schema>.error_code
(
  aCaption <schema>.error.caption%TYPE
)
RETURNS <schema>.error.error_id%TYPE AS
$BODY$

DECLARE
  mErrorCode <schema>.error.error_id%TYPE;

BEGIN

  mErrorCode := <schema>.id_from_caption('error', aCaption);

  IF mErrorCode < 1 THEN
    RAISE WARNING 'Error key "%" not found.', aCaption;
    RETURN <schema>.error_code('ErrorNF');
  END IF;

  RETURN -mErrorCode;

END
$BODY$
LANGUAGE PLPGSQL IMMUTABLE; --STABLE; -- I think it could be IMMUTABLE because we not change the error table
                                      -- after Filu was installed
--
-- END OF FUNCTION <schema>.error_code
--
