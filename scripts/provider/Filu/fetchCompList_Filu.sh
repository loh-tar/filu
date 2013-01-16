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
  echo "  ./me <Symbol>"
  echo "  ./me DAX"
  exit 0;
  ;;

  "--info" )
  echo "Name:    fetchCompList_Filu"
  echo "Purpose: To illustrate how could be all associated underlying to an Index with RefSymbol and Weight fetched"
  echo "Input:   Symbol"
  echo "Output:  RefSymbol Weight"
  echo "Comment: I know only the two indexes 'DAX' and 'NDX' with outdated demo data"
  exit 0
  ;;

esac

echo "[Header]RefSymbol;Weight"

case "$1" in
  "DAX"|"^GDAXI" )
  cat Index_DAX_Filu.csv
  ;;

  "NDX"|"^NDX" )
  cat Index_NASDAQ100_Filu.csv
  ;;

esac

exit 0
