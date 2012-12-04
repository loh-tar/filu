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
--
-- These DataTypes are used by fpi_foo functions as handy return type.
-- Because Filu indicators, see doc/indicator-file-format.txt, are always
-- calculates with C++ double type, you will find here no bool, int or
-- other data type.
-- When you add more types follow the existing naming convention, which
-- meens each type name begins with "f" just as each includet type.
--
-- Double Type
--
DROP TYPE IF EXISTS :filu.fdouble CASCADE;

CREATE TYPE :filu.fdouble AS(
    fdate     date,
    ftime     time,
    fdata     float
);
--
--
-- Data Types With Two Return Values
--
-- Double2 Type
--
DROP TYPE IF EXISTS :filu.fdouble2 CASCADE;

CREATE TYPE :filu.fdouble2 AS(
    fdate     date,
    ftime     time,
    fdata1    float,
    fdata2    float
);
--
-- *
-- *
-- *  Create more types if needed
-- *
-- *
--
--
-- Data Types With Special Data Names
--
-- Bar Type
--
DROP TYPE IF EXISTS :filu.fbar CASCADE;

CREATE TYPE :filu.fbar AS(
    fdate     date,
    ftime     time,
    fopen     float,
    fhigh     float,
    flow      float,
    fclose    float,
    fvol      float,
    foi       float
);
--
-- END OF FILE data_types.sql
--
