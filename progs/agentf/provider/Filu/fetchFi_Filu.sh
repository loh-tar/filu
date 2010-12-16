#!/bin/bash

echo "[Header]RefSymbol;Name;Type;Provider0;Symbol0;Market0;Provider1;Symbol1;Market1"
grep -i -E $1 all_FIs_Filu.csv
exit 0
