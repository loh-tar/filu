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
