#!/bin/bash

case "$1" in
  "" )
  echo "Call me this way:"
  echo "  ./me <Symbol>"
  echo "  ./me DAX"
  exit 0;
  ;;

  "--info" )
  echo "Name:    fetchCompList_Filu"
  echo "Purpose: Fetch all associated underlying to an Index with RefSymbol and Weight."
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
