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
--
--
-- Non Provider Symbol Types
--
INSERT INTO <schema>.stype(caption, seq) values('Reuters', 100);
INSERT INTO <schema>.stype(caption, seq) values('ISIN', 1100);
--INSERT INTO <schema>.stype(caption, seq) values('ISO', 300);
--INSERT INTO <schema>.stype(caption, seq) values('WKN', 1000);
--INSERT INTO <schema>.stype(caption, seq) values('Generic', 500);
--
--
-- Provider Symbol Types
--
INSERT INTO <schema>.stype(caption, seq, isprovider) values('Yahoo', 200, true);
--INSERT INTO <schema>.stype(caption, seq, isprovider) values('futuresguide', 1100, true);
