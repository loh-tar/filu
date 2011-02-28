/*
 *   This file is part of Filu.
 *
 *   Copyright (C) 2007, 2010, 2011  loh.tar@googlemail.com
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
-- Usual Error Messages
--
INSERT INTO <schema>.error(caption, etext) VALUES('ForeignKV', 'Foreign key violation.');
INSERT INTO <schema>.error(caption, etext) VALUES('PrimaryKeyNF', 'Given primary key (the ID) not found.');
INSERT INTO <schema>.error(caption, etext) VALUES('QualityToBad', 'Quality is not good enough for an update.');
INSERT INTO <schema>.error(caption, etext) VALUES('UniqueV', 'Unique violation.');
INSERT INTO <schema>.error(caption, etext) VALUES('UnkownErr', 'Oops, unknown error.');
--INSERT INTO <schema>.error(caption, etext) VALUES('', '.');
--
--
-- FI Types
--
-- Don't remove Currency, it's a must have!
INSERT INTO <schema>.ftype(caption) VALUES('Currency');
--
INSERT INTO <schema>.ftype(caption) VALUES('Stock');
INSERT INTO <schema>.ftype(caption) VALUES('Index');
--INSERT INTO <schema>.ftype(caption) VALUES('Bond');
--INSERT INTO <schema>.ftype(caption) VALUES('FutureMaster');
--INSERT INTO <schema>.ftype(caption) VALUES('Future');
--INSERT INTO <schema>.ftype(caption) VALUES('Option');
--INSERT INTO <schema>.ftype(caption) VALUES('Warrant');
--INSERT INTO <schema>.ftype(caption) VALUES('IRS');
--INSERT INTO <schema>.ftype(caption) VALUES('CRS');
--
--
-- Non Provider Symbol Types
--
-- Don't remove Reuters, it's a must have!
INSERT INTO <schema>.stype(caption, seq) VALUES('Reuters', 100);
--
INSERT INTO <schema>.stype(caption, seq) VALUES('ISIN', 1100);
--INSERT INTO <schema>.stype(caption, seq) VALUES('ISO', 300);
--INSERT INTO <schema>.stype(caption, seq) VALUES('WKN', 1000);
--INSERT INTO <schema>.stype(caption, seq) VALUES('Generic', 500);
--
--
-- Provider Symbol Types
--
INSERT INTO <schema>.stype(caption, seq, isprovider) VALUES('Yahoo', 200, true);
--INSERT INTO <schema>.stype(caption, seq, isprovider) VALUES('futuresguide', 1100, true);
--
--
-- Markets
--
-- Don't remove Currency or change the symbol USD, it's a must have!
SELECT market_insert('Currency', 'US Dollar', 'USD', -1); -- "-1" Says "That's the NoMarket insert"
--
--SELECT market_insert('NYSE', '', 'USD'); -- No need to give name, USD already exist
--SELECT market_insert('Xetra', 'Euro', 'EUR');
--
