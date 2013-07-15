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
--
-- Usual Error Messages
--
INSERT INTO :filu.error(caption, etext) VALUES('ForeignKV', 'Foreign key violation.');
INSERT INTO :filu.error(caption, etext) VALUES('PrimaryKeyNF', 'Given primary key (the ID) not found.');
INSERT INTO :filu.error(caption, etext) VALUES('QualityToBad', 'Quality is not good enough for an update.');
INSERT INTO :filu.error(caption, etext) VALUES('UniqueV', 'Unique violation.');
INSERT INTO :filu.error(caption, etext) VALUES('UnkownErr', 'Oops, unknown error.');
--INSERT INTO :filu.error(caption, etext) VALUES('', '.');
--
--
-- FI Types
--
-- Don't remove Currency, it's a must have!
INSERT INTO :filu.ftype(caption) VALUES('Currency');
--
INSERT INTO :filu.ftype(caption) VALUES('Stock');
INSERT INTO :filu.ftype(caption) VALUES('Index');
--INSERT INTO :filu.ftype(caption) VALUES('Bond');
--INSERT INTO :filu.ftype(caption) VALUES('FutureMaster');
--INSERT INTO :filu.ftype(caption) VALUES('Future');
--INSERT INTO :filu.ftype(caption) VALUES('Option');
--INSERT INTO :filu.ftype(caption) VALUES('Warrant');
--INSERT INTO :filu.ftype(caption) VALUES('IRS');
--INSERT INTO :filu.ftype(caption) VALUES('CRS');
--
--
-- Provider Symbol Types
--
-- The very first entry *must* be the currency quote provider.
-- Symbols taken by market_insert() are treated as symbol of the
-- currency provider and expected to have stype_id=1
INSERT INTO :filu.stype(caption, seq, isprovider) VALUES('ECB', 100, true);
--
INSERT INTO :filu.stype(caption, seq, isprovider) VALUES('Yahoo', 200, true);
--INSERT INTO :filu.stype(caption, seq, isprovider) VALUES('futuresguide', 1100, true);
--
--
-- Non Provider Symbol Types
--
-- Don't remove Reuters, it's a must have! Expected e.g. by market_insert()
INSERT INTO :filu.stype(caption, seq) VALUES('Reuters', 100);
--
INSERT INTO :filu.stype(caption, seq) VALUES('ISIN', 1100);
--INSERT INTO :filu.stype(caption, seq) VALUES('ISO', 300);
--INSERT INTO :filu.stype(caption, seq) VALUES('WKN', 1000);
--INSERT INTO :filu.stype(caption, seq) VALUES('Generic', 500);
--
--
-- Markets
--
-- Don't remove the first entry, it's a must have! But you can adjust it to your neeeds.
--   The given currency is the base currency which is used to represent other currencys.
--   Our currency qoute provider, the European Central Bank, use the euro as base.
--   See also: http://en.wikipedia.org/wiki/Currency_pair
SELECT :filu.market_insert('Forex', 'Euro', 'EUR', -1);        -- "-1" Says "That's the NoMarket insert"
--SELECT :filu.market_insert('Forex', 'US Dollar', 'USD', -1); -- "-1" Says "That's the NoMarket insert"
--
--
--SELECT :filu.market_insert('NewYork', 'US Dollar', 'USD');
--SELECT :filu.market_insert('Xetra', 'Euro', 'EUR');
--
