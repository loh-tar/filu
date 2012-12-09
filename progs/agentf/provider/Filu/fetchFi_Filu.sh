#!/bin/bash

case "$1" in
  "" )
  echo "Call me this way:"
  echo "  ./me <Symbol>"
  echo "  ./me DAX"
  exit 0;
  ;;

  "--info" )
  echo "Name:    fetchFi_Filu"
  echo "Purpose: Fetch the master data to a FI"
  echo "Input:   SearchString"
  echo "Output:  Name Type <SymbolType> [RefSymbol]"
  echo "Comment: The SearchString can be a part of a symbol or FI name"
  echo "Comment: I know only few FIs with outdated demo data"
  exit 0
  ;;

esac

echo "[Header]RefSymbol;Name;Type;Provider0;Symbol0;Market0;Provider1;Symbol1;Market1"

# http://stackoverflow.com/questions/59895/can-a-bash-script-tell-what-directory-its-stored-in
DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

grep -i -E $1 $DIR/all_FIs_Filu.csv

exit 0
