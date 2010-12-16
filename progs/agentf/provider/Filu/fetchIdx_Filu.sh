#!/bin/bash

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
