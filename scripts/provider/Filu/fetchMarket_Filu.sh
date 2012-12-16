#!/bin/bash

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
