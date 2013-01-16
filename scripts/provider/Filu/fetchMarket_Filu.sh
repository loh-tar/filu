#!/bin/bash
#
#    This file is part of Filu.
#
#    Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
#
#    Filu is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 2 of the License, or
#    (at your option) any later version.
#
#    Filu is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Filu. If not, see <http://www.gnu.org/licenses/>.
#

case "$1" in
  "" )
  echo "Call me this way:"
  echo "  ./me <Pattern>"
  echo "  ./me nyse"
  exit 0;
  ;;

  "--info" )
  echo "Name:    fetchMarket_Filu"
  echo "Purpose: To illustrate how could be markets fetched"
  echo "Input:   SearchString"
  echo "Output:  MarketName MarketSymbol Currency CurrencySymbol OpenTime CloseTime Lunch Location"
  echo "Comment: Knows only a few markets"
  echo "Comment: The SearchString can be part of the market or currency"
  exit 0
  ;;

esac

echo "[Header]MarketName;MarketSymbol;Currency;CurrencySymbol;OpenTime;CloseTime;Lunch;Location"

# http://stackoverflow.com/questions/59895/can-a-bash-script-tell-what-directory-its-stored-in
DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

grep -i -E $1 $DIR/all_Markets_Filu.csv

exit 0
